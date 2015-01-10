#include <hide/Indexer.h>

#include <boost/filesystem.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/scope_exit.hpp>

#include <hide/utils/ListenersHolder.h>
#include <hide/utils/PTree.h>
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
			_inst->_events.push(Event(EventType::IndexableAdded, file));
			_inst->_condVar.notify_all();
		}

		virtual void OnFileRemoved(const IFilePtr& file)
		{
			HIDE_LOCK(_inst->_mutex);
			_inst->_events.push(Event(EventType::IndexableRemoved, file));
			_inst->_condVar.notify_all();
		}

		virtual void OnFileModified(const IFilePtr& file)
		{
			OnFileRemoved(file);
			OnFileAdded(file);
		}
	};


	////////////////////////////////////////////////////////////////////////////////


	class Indexer::PartialIndexInfo
	{
	private:
		IPartialIndexPtr			_index;
		boost::filesystem::path		_indexFilePath;
		boost::filesystem::path		_metaFilePath;

	public:
		PartialIndexInfo(const IPartialIndexPtr& index, const boost::filesystem::path& indexFilePath, const boost::filesystem::path& metaFilePath)
			: _index(index), _indexFilePath(indexFilePath), _metaFilePath(metaFilePath)
		{ }

		IPartialIndexPtr GetIndex() const					{ return _index; }
		boost::filesystem::path GetIndexFilePath() const	{ return _indexFilePath; }
		boost::filesystem::path GetMetaFilePath() const		{ return _metaFilePath; }
	};


	////////////////////////////////////////////////////////////////////////////////


	class PartialIndexMetaInfo
	{
	private:
		Time		_modificationTime;

	public:
		PartialIndexMetaInfo()
		{ }

		PartialIndexMetaInfo(Time modificationTime)
			: _modificationTime(modificationTime)
		{ }

		Time GetModificationTime() const { return _modificationTime; }

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

		HIDE_DECLARE_MEMBERS("modificationTime", &PartialIndexMetaInfo::_modificationTime);
	};


	////////////////////////////////////////////////////////////////////////////////


	class Indexer::IndexQueryInfo : public ListenersHolder<IIndexQueryListener, IIndexQuery>
	{
		typedef std::function<bool(const IIndexEntryPtr&)>		CheckEntryFunc;
		typedef std::vector<IndexQueryEntry>					Entries;

	private:
		CheckEntryFunc	_checkEntryFunc;
		Entries			_entries;
		bool			_finished;

	public:
		IndexQueryInfo(const CheckEntryFunc& checkEntryFunc)
			: _checkEntryFunc(checkEntryFunc), _finished(false)
		{ }

		void Interrupt()
		{
			// TODO: implement
		}

		void Process(const Indexer* indexer)
		{
			for (auto pi : indexer->_partialIndexes)
				for (auto e : pi.second->GetIndex()->GetEntries())
					if (_checkEntryFunc(e))
					{
						IndexQueryEntry entry(e->GetFullName(), e->GetLocation());
						HIDE_LOCK(GetMutex());
						_entries.push_back(entry);
						InvokeListeners(std::bind(&IIndexQueryListener::OnEntry, std::placeholders::_1, entry));
					}

			HIDE_LOCK(GetMutex());
			_finished = true;
			InvokeListeners(std::bind(&IIndexQueryListener::OnFinished, std::placeholders::_1));
		}

	protected:
		virtual void PopulateState(const IIndexQueryListenerPtr& listener) const
		{
			for (auto e : _entries)
				listener->OnEntry(e);

			if (_finished)
				listener->OnFinished();
		}
	};


	////////////////////////////////////////////////////////////////////////////////


	class Indexer::IndexQuery : public virtual IIndexQuery
	{
	private:
		IndexQueryInfoPtr		_queryInfo;

	public:
		IndexQuery(const IndexQueryInfoPtr queryInfo)
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
		_events.push(Event(EventType::RemoveOutdatedFiles, nullptr)); // Now that we have all the events from the ProjectFiles::PopulateState
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
		return DoQuerySymbols([str](const IIndexEntryPtr& e) { return e->GetName().find(str) != std::string::npos; });
	}

	IIndexQueryPtr Indexer::QuerySymbolsByName(const std::string& symbolName)
	{
		s_logger.Info() << "QuerySymbolsByName(" << symbolName << ")";
		return DoQuerySymbols([symbolName](const IIndexEntryPtr& e) { return e->GetName() == symbolName; });
	}


	void Indexer::ThreadFunc()
	{
		std::unique_lock<std::mutex> l(_mutex);

		bool scanning_indexes = false;

		while (_working)
		{
			if (!scanning_indexes && !_events.empty())
			{
				scanning_indexes = true;
				s_logger.Info() << "Scanning indexes...";
			}
			else if (scanning_indexes && _events.empty())
			{
				scanning_indexes = false;
				s_logger.Info() << "Finished scanning indexes";
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

				IFilePtr file;
				switch (e.Type.GetRaw())
				{
				case EventType::IndexableAdded:
					AddIndexable(e.Indexable);
					break;
				case EventType::IndexableRemoved:
					RemoveIndexable(e.Indexable);
					break;
				case EventType::RemoveOutdatedFiles:
					RemoveOutdatedFiles();
					break;
				}
			}
			else if (!_queries.empty())
			{
				IndexQueryInfoPtr q = _queries.front();
				_queries.pop();
				q->Process(this);
			}
		}
	}


	IIndexQueryPtr Indexer::DoQuerySymbols(const std::function<bool(const IIndexEntryPtr&)>& checkEntryFunc)
	{
		IndexQueryInfoPtr query_info(new IndexQueryInfo(checkEntryFunc));
		HIDE_LOCK(_mutex);
		_queries.push(query_info);
		_condVar.notify_all();
		return std::make_shared<IndexQuery>(query_info);
	}


	void Indexer::AddIndexable(const IIndexablePtr& indexable)
	{
		using namespace boost::filesystem;

		IIndexableIdPtr id = indexable->GetIndexableId();

		path index_file = GetIndexFilePath(indexable);
		path meta_file = GetMetaFilePath(indexable);

		if (is_regular_file(meta_file))
		{
			PartialIndexMetaInfo old_meta_info;
			try
			{
				old_meta_info = PartialIndexMetaInfo::LoadFromFile(meta_file);

				if (old_meta_info.GetModificationTime() == indexable->GetModificationTime() && is_regular_file(index_file))
				{
					try
					{
						s_logger.Debug() << "Loading " << indexable << " index from " << index_file;
						IPartialIndexPtr index = indexable->GetIndexer()->LoadIndex(index_file.string());
						{
							HIDE_LOCK(_mutex);
							AddPartialIndex(id, PartialIndexInfoPtr(new PartialIndexInfo(index, index_file, meta_file)));
						}
						return;
					}
					catch (const std::exception& ex)
					{ s_logger.Warning() << "Cannot load " << indexable << " index: " << boost::diagnostic_information(ex); }
				}
			}
			catch (const std::exception& ex)
			{ s_logger.Warning() << "Cannot read " << indexable << " metainfo: " << boost::diagnostic_information(ex); }
		}

		s_logger.Info() << "Building " << indexable << " index";
		IPartialIndexPtr index = indexable->GetIndexer()->BuildIndex();
		{
			HIDE_LOCK(_mutex);
			AddPartialIndex(id, PartialIndexInfoPtr(new PartialIndexInfo(index, index_file, meta_file)));
		}

		s_logger.Debug() << "Saving " << indexable << " index to " << index_file;
		boost::filesystem::create_directories(index_file.parent_path());
		index->Save(index_file.string());
		PartialIndexMetaInfo(indexable->GetModificationTime()).SaveToFile(meta_file);
	}


	void Indexer::RemoveIndexable(const IIndexablePtr& indexable)
	{
		using namespace boost::filesystem;

		IIndexableIdPtr id = indexable->GetIndexableId();
		auto it = _partialIndexes.find(id);
		if (it == _partialIndexes.end())
			return;

		DoRemoveFile(it->second->GetIndexFilePath());
		DoRemoveFile(it->second->GetMetaFilePath());
		RemovePartialIndex(id);
	}


	void Indexer::RemoveOutdatedFiles()
	{
		using namespace boost::filesystem;

		s_logger.Info() << "RemoveOutdatedFiles()";

		HIDE_LOCK(_mutex);
		std::set<path> good_files;
		for (auto pi : _partialIndexes)
		{
			good_files.insert(path(pi.second->GetIndexFilePath()).normalize());
			good_files.insert(path(pi.second->GetMetaFilePath()).normalize());
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
					DoRemoveFile(p);
			}
		};

		scan_func(s_indexDirectory);
	}


	void Indexer::AddPartialIndex(const IIndexableIdPtr& indexableId, const PartialIndexInfoPtr& partialIndexInfo)
	{
		_partialIndexes.insert(std::make_pair(indexableId, partialIndexInfo));
	}


	void Indexer::RemovePartialIndex(const IIndexableIdPtr& indexableId)
	{
		_partialIndexes.erase(indexableId);
	}


	void Indexer::DoRemoveFile(const boost::filesystem::path& filepath)
	{
		using namespace boost::filesystem;

		boost::system::error_code err;

		s_logger.Info() << "Removing " << filepath;
		remove(filepath, err);
		if (err)
			s_logger.Warning() << "Could not remove " << filepath << ": " << err;

		path p = filepath.parent_path(), index_dir = s_indexDirectory;
		p.normalize();
		index_dir.normalize();
		for (; p != index_dir && is_empty(p); p = p.parent_path())
		{
			s_logger.Info() << "Removing " << p;
			remove(p, err);
			if (err)
				s_logger.Warning() << "Could not remove " << p << ": " << err;
		}
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
