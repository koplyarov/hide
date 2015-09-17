#include <hide/Indexer.h>

#include <set>

#include <boost/filesystem.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/scope_exit.hpp>

#include <hide/utils/FileSystemUtils.h>
#include <hide/utils/ListenersHolder.h>
#include <hide/utils/PTree.h>
#include <hide/utils/Profiler.h>
#include <hide/utils/Thread.h>


namespace hide
{


	class Indexer::FilesListener : public virtual IProjectFilesListener
	{
	private:
		Indexer*		_inst;

	public:
		FilesListener(Indexer* inst) : _inst(inst) { }

		virtual void OnFileAdded(const IFilePtr& file)
		{
			HIDE_LOCK(_inst->_mutex);
			_inst->_events.push(Event{EventType::IndexableAdded, file});
			_inst->_condVar.notify_all();
		}

		virtual void OnFileRemoved(const IFilePtr& file)
		{
			HIDE_LOCK(_inst->_mutex);
			_inst->_events.push(Event{EventType::IndexableRemoved, file});
			_inst->_condVar.notify_all();
		}

		virtual void OnFileModified(const IFilePtr& file)
		{
			HIDE_LOCK(_inst->_mutex);
			_inst->_events.push(Event{EventType::IndexableModified, file});
			_inst->_condVar.notify_all();
		}
	};


	////////////////////////////////////////////////////////////////////////////////


	struct Indexer::PartialIndexInfo
	{
		typedef std::vector<StringToIndexEntry::iterator>		StringToIndexEntryIterators;

		IPartialIndexPtr				Index;
		boost::filesystem::path			IndexFilePath;
		boost::filesystem::path			MetaFilePath;
		StringToIndexEntryIterators		SymbolNameToSymbolIterators;
		StringToIndexEntryIterators		FullSymbolNameToSymbolIterators;
	};


	////////////////////////////////////////////////////////////////////////////////


	struct PartialIndexMetaInfo
	{
		Time		ModificationTime;

		void SaveToFile(const boost::filesystem::path& p) const
		{
			boost::property_tree::ptree root;
			PTreeWriter w(root);
			w.Write("metaInfo", *this);
			std::ofstream f(p.string(), std::ios_base::trunc);
			boost::property_tree::write_json(f, root, false);
		}

		static PartialIndexMetaInfo LoadFromFile(const boost::filesystem::path& p)
		{
			boost::property_tree::ptree root;
			std::ifstream f(p.string());
			boost::property_tree::read_json(f, root);
			PartialIndexMetaInfo result;
			PTreeReader(root).Read("metaInfo", result);
			return result;
		}

		HIDE_DECLARE_MEMBERS("modificationTime", &PartialIndexMetaInfo::ModificationTime);
	};


	////////////////////////////////////////////////////////////////////////////////


	class Indexer::IndexQueryInfoBase : public ListenersHolder<IIndexQueryListener, IIndexQuery>
	{
		typedef std::vector<IndexQueryEntry>					Entries;

	private:
		Entries			_entries;
		bool			_finished;

	public:
		virtual ~IndexQueryInfoBase() { }

		void Interrupt()
		{
			// TODO: implement
		}

		virtual void Process(const Indexer* indexer) = 0;

	protected:
		IndexQueryInfoBase() : _finished(false) { }

		void ReportEntry(const IndexQueryEntry& entry)
		{
			HIDE_LOCK(GetMutex());
			_entries.push_back(entry);
			InvokeListeners(std::bind(&IIndexQueryListener::OnEntry, std::placeholders::_1, entry));
		}

		void ReportFinished()
		{
			HIDE_LOCK(GetMutex());
			_finished = true;
			InvokeListeners(std::bind(&IIndexQueryListener::OnFinished, std::placeholders::_1));
		}

		virtual void PopulateState(const IIndexQueryListenerPtr& listener) const
		{
			for (const auto& e : _entries)
				listener->OnEntry(e);

			if (_finished)
				listener->OnFinished();
		}
	};


	class Indexer::GenericIndexQueryInfo : public Indexer::IndexQueryInfoBase
	{
		typedef std::function<bool(const IIndexEntryPtr&)>		CheckEntryFunc;

	private:
		CheckEntryFunc	_checkEntryFunc;

	public:
		GenericIndexQueryInfo(const CheckEntryFunc& checkEntryFunc)
			: _checkEntryFunc(checkEntryFunc)
		{ }

		virtual void Process(const Indexer* indexer)
		{
			for (const auto& pi : indexer->_partialIndexes)
				for (const auto& e : pi.second->Index->GetEntries())
					if (_checkEntryFunc(e))
						ReportEntry(IndexQueryEntry(e->GetFullName(), e->GetLocation(), e->GetKind()));

			ReportFinished();
		}
	};


	class Indexer::FastIndexQueryInfo : public Indexer::IndexQueryInfoBase
	{
		typedef StringToIndexEntry Indexer::*StringToIndexEntryMember;

	private:
		StringToIndexEntryMember	_stringToIndexEntryMember;
		std::string					_string;

	public:
		FastIndexQueryInfo(const StringToIndexEntryMember& stringToIndexEntryMember, const std::string& string)
			: _stringToIndexEntryMember(stringToIndexEntryMember), _string(string)
		{ }

		virtual void Process(const Indexer* indexer)
		{
			auto range = (indexer->*_stringToIndexEntryMember).equal_range(_string);
			for (auto it = range.first; it != range.second; ++it)
				ReportEntry(IndexQueryEntry(it->second->GetFullName(), it->second->GetLocation(), it->second->GetKind()));

			ReportFinished();
		}
	};


	////////////////////////////////////////////////////////////////////////////////


	class Indexer::IndexQuery : public virtual IIndexQuery
	{
	private:
		IndexQueryInfoBasePtr		_queryInfo;

	public:
		IndexQuery(const IndexQueryInfoBasePtr& queryInfo)
			: _queryInfo(queryInfo)
		{ }

		~IndexQuery()
		{ _queryInfo->Interrupt(); }

		virtual void AddListener(const IIndexQueryListenerPtr& listener)	{ _queryInfo->AddListener(listener); }
		virtual void RemoveListener(const IIndexQueryListenerPtr& listener)	{ _queryInfo->RemoveListener(listener); }
	};


	////////////////////////////////////////////////////////////////////////////////


	HIDE_NAMED_LOGGER(Indexer);

	const std::string Indexer::s_indexDirectory(".hide/index");

	Indexer::Indexer(const ProjectFilesPtr& files)
		: _working(true), _files(files)
	{
		_filesListener = std::make_shared<FilesListener>(this);
		_files->AddListener(_filesListener);
		{
			HIDE_LOCK(_mutex);
			_events.push(Event{EventType::RemoveOutdatedFiles, nullptr}); // Now that we have all the events from the ProjectFiles::PopulateState
		}
		_thread = MakeThread("indexer", std::bind(&Indexer::ThreadFunc, this));
	}


	Indexer::~Indexer()
	{
		{
			HIDE_LOCK(_mutex);
			_working = false;
			_condVar.notify_all();
		}
		_thread.join();
		_files->RemoveListener(_filesListener);
		_filesListener.reset();
	}


	IIndexQueryPtr Indexer::QuerySymbolsBySubstring(const std::string& str)
	{
		s_logger.Info() << "QuerySymbolsBySubstring(" << str << ")";
		return DoQuerySymbols(std::make_shared<GenericIndexQueryInfo>([str](const IIndexEntryPtr& e) { return e->GetName().find(str) != std::string::npos; }));
	}

	IIndexQueryPtr Indexer::QuerySymbolsByName(const std::string& symbolName)
	{
		s_logger.Info() << "QuerySymbolsByName(" << symbolName << ")";
		return DoQuerySymbols(std::make_shared<FastIndexQueryInfo>(&Indexer::_symbolNameToSymbol, symbolName));
	}


	void Indexer::PopulateState(const IIndexerListenerPtr& listener) const
	{
		Diff<IIndexEntryPtr>::ElementArray added;
		for (const auto& s_pair : _symbolNameToSymbol)
			added.push_back(s_pair.second);
		listener->OnIndexChanged(Diff<IIndexEntryPtr>(added, {}));
	}


	void Indexer::ThreadFunc()
	{
		std::unique_lock<std::mutex> l(_mutex);

		bool scanning_indexes = false;
		Profiler<> scan_profiler;

		while (_working)
		{
			if (!scanning_indexes && !_events.empty())
			{
				scanning_indexes = true;
				s_logger.Debug() << "Scanning indexes...";
				scan_profiler.Reset();
			}
			else if (scanning_indexes && _events.empty())
			{
				scanning_indexes = false;
				auto scan_duration = scan_profiler.Reset();
				(scan_duration > std::chrono::seconds(1) ? s_logger.Info() : s_logger.Debug()) << "Finished scanning indexes: " << scan_duration;
			}

			if (_events.empty() && _queries.empty())
			{
				_condVar.wait(l);
				continue;
			}

			if (!_events.empty())
			{
				Event e = _events.front();
				_events.pop();

				l.unlock();
				BOOST_SCOPE_EXIT_ALL(&) { l.lock(); };

				try
				{
					switch (e.Type.GetRaw())
					{
					case EventType::IndexableAdded:
					case EventType::IndexableRemoved:
					case EventType::IndexableModified:
						ProcessIndexableEvent(e);
						break;
					case EventType::RemoveOutdatedFiles:
						RemoveOutdatedFiles();
						break;
					}
				}
				catch (const std::exception& ex)
				{ s_logger.Error() << "An exception while processing an event " << e << ": " << ex; }
			}
			else if (!_queries.empty())
			{
				IndexQueryInfoBasePtr q = _queries.front();
				_queries.pop();

				try
				{
					Profiler<> query_profiler;
					HIDE_LOCK(GetMutex());
					q->Process(this);
					Indexer::s_logger.Warning() << "Query processing time: " << query_profiler.Reset();
				}
				catch (const std::exception& ex)
				{ s_logger.Error() << "An exception while processing a query: " << ex; }
			}
		}
	}


	IIndexQueryPtr Indexer::DoQuerySymbols(const IndexQueryInfoBasePtr& queryInfo)
	{
		HIDE_LOCK(_mutex);
		_queries.push(queryInfo);
		_condVar.notify_all();
		return std::make_shared<IndexQuery>(queryInfo);
	}


	void Indexer::ProcessIndexableEvent(const Event& e)
	{
		using namespace boost::filesystem;

		IIndexablePtr indexable = e.Indexable;

		IIndexableIdPtr id = indexable->GetIndexableId();

		path index_file = GetIndexFilePath(indexable);
		path meta_file = GetMetaFilePath(indexable);

		IPartialIndexPtr index;

		if (e.Type == EventType::IndexableAdded || e.Type == EventType::IndexableModified)
		{
			if (is_regular_file(meta_file))
			{
				PartialIndexMetaInfo old_meta_info;
				try
				{
					old_meta_info = PartialIndexMetaInfo::LoadFromFile(meta_file);

					if (old_meta_info.ModificationTime == indexable->GetModificationTime() && is_regular_file(index_file))
					{
						try
						{
							s_logger.Debug() << "Loading " << indexable << " index from " << index_file;
							index = indexable->GetIndexer()->LoadIndex(index_file.string());
						}
						catch (const std::exception& ex)
						{ s_logger.Warning() << "Cannot load " << indexable << " index: " << ex; }
					}
				}
				catch (const std::exception& ex)
				{ s_logger.Warning() << "Cannot read " << indexable << " metainfo: " << ex; }
			}

			if (!index)
			{
				s_logger.Debug() << "Building " << indexable << " index";
				index = indexable->GetIndexer()->BuildIndex();

				s_logger.Debug() << "Saving " << indexable << " index to " << index_file;
				boost::filesystem::create_directories(index_file.parent_path());
				index->Save(index_file.string());
				PartialIndexMetaInfo{indexable->GetModificationTime()}.SaveToFile(meta_file);
			}
		}

		typedef std::set<IIndexEntryPtr, Less>	IndexEntriesSet;
		IndexEntriesSet old_entries;
		IndexEntriesSet new_entries;

		HIDE_LOCK(GetMutex());

		if (e.Type == EventType::IndexableRemoved || e.Type == EventType::IndexableModified)
		{

			IIndexableIdPtr id = indexable->GetIndexableId();
			auto indexable_it = _partialIndexes.find(id);
			if (indexable_it == _partialIndexes.end())
				return;

			RemoveFileAndParentDirectories(indexable_it->second->IndexFilePath, s_indexDirectory);
			RemoveFileAndParentDirectories(indexable_it->second->MetaFilePath, s_indexDirectory);

			for (auto it : indexable_it->second->SymbolNameToSymbolIterators)
			{
				old_entries.insert(it->second);
				_symbolNameToSymbol.erase(it);
			}

			for (auto it : indexable_it->second->FullSymbolNameToSymbolIterators)
				_fullSymbolNameToSymbol.erase(it);

			_partialIndexes.erase(indexable_it);
		}
		if (e.Type == EventType::IndexableAdded || e.Type == EventType::IndexableModified)
		{
			PartialIndexInfo::StringToIndexEntryIterators symbol_name_to_symbol_iterators;
			PartialIndexInfo::StringToIndexEntryIterators full_symbol_name_to_symbol_iterators;

			IIndexEntryPtrArray entries = index->GetEntries();
			symbol_name_to_symbol_iterators.reserve(entries.size());
			full_symbol_name_to_symbol_iterators.reserve(entries.size());

			for (const auto& e : entries)
			{
				symbol_name_to_symbol_iterators.push_back(_symbolNameToSymbol.insert(std::make_pair(e->GetName(), e)));
				new_entries.insert(e);
				full_symbol_name_to_symbol_iterators.push_back(_fullSymbolNameToSymbol.insert(std::make_pair(e->GetFullName(), e)));
			}

			PartialIndexInfoPtr index_info(new PartialIndexInfo{index, index_file, meta_file, std::move(symbol_name_to_symbol_iterators), std::move(full_symbol_name_to_symbol_iterators)});
			_partialIndexes.insert(std::make_pair(id, index_info));
		}

		IndexEntriesSet added_entries;
		IndexEntriesSet removed_entries;

		boost::set_difference(old_entries, new_entries, std::inserter(removed_entries, removed_entries.begin()), Less());
		boost::set_difference(new_entries, old_entries, std::inserter(added_entries, added_entries.begin()), Less());

		InvokeListeners(std::bind(&IIndexerListener::OnIndexChanged, std::placeholders::_1, Diff<IIndexEntryPtr>(Diff<IIndexEntryPtr>::ElementArray(added_entries.begin(), added_entries.end()), Diff<IIndexEntryPtr>::ElementArray(removed_entries.begin(), removed_entries.end()))));
	}


	void Indexer::RemoveOutdatedFiles()
	{
		using namespace boost::filesystem;

		Profiler<> profiler;
		s_logger.Debug() << "RemoveOutdatedFiles()";

		HIDE_LOCK(GetMutex());
		std::set<path> good_files;
		for (const auto& pi : _partialIndexes)
		{
			good_files.insert(path(pi.second->IndexFilePath).normalize());
			good_files.insert(path(pi.second->MetaFilePath).normalize());
		}

		std::function<void(const path&)> scan_func = [&](const path& p)
		{
			using namespace boost::filesystem;

			if (is_directory(p))
			{
				directory_iterator end;
				for (directory_iterator it(p); it != end; ++it)
					scan_func(*it);
			}
			else
			{
				if (good_files.find(path(p).normalize()) == good_files.end())
					RemoveFileAndParentDirectories(p, s_indexDirectory);
			}
		};

		s_logger.Info() << "RemoveOutdatedFiles finished: " << profiler.Reset();

		scan_func(s_indexDirectory);
	}


	boost::filesystem::path Indexer::GetIndexFilePath(const IIndexablePtr& indexable)
	{
		using namespace boost::filesystem;

		auto file = std::dynamic_pointer_cast<IFile>(indexable);
		HIDE_CHECK(file, std::runtime_error("Not implemented!"));
		return path(s_indexDirectory) / path(file->GetFilename() + ".index");
	}


	boost::filesystem::path Indexer::GetMetaFilePath(const IIndexablePtr& indexable)
	{
		using namespace boost::filesystem;

		auto file = std::dynamic_pointer_cast<IFile>(indexable);
		HIDE_CHECK(file, std::runtime_error("Not implemented!"));
		return path(s_indexDirectory) / path(file->GetFilename() + ".meta");
	}

}
