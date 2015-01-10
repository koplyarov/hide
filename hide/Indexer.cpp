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
				for (auto e : pi.second->GetEntries())
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


	HIDE_NAMED_LOGGER(Indexer);

	Indexer::Indexer(const ProjectFilesPtr& files)
		: _working(true), _files(files)
	{
		_filesListener = std::make_shared<FilesListener>(this);
		_files->AddListener(_filesListener);
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

		while (_working)
		{
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
					file = std::dynamic_pointer_cast<IFile>(e.Indexable);
					HIDE_CHECK(file, std::runtime_error("Not implemented!"));
					OnFileAdded(file);
					break;
				case EventType::IndexableRemoved:
					file = std::dynamic_pointer_cast<IFile>(e.Indexable);
					HIDE_CHECK(file, std::runtime_error("Not implemented!"));
					OnFileRemoved(file);
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


	void Indexer::OnFileAdded(const IFilePtr& file)
	{
		using namespace boost::filesystem;

		IIndexableIdPtr id = file->GetIndexableId();

		path index_file(path(".hide/index") / path(file->GetFilename() + ".index"));
		path meta_file(path(".hide/index") / path(file->GetFilename() + ".meta"));

		if (is_regular_file(meta_file))
		{
			PartialIndexMetaInfo old_meta_info;
			try
			{
				old_meta_info = PartialIndexMetaInfo::LoadFromFile(meta_file);

				if (old_meta_info.GetModificationTime() == file->GetModificationTime() && is_regular_file(index_file))
				{
					try
					{
						s_logger.Debug() << "Loading " << file << " index from " << index_file;
						IPartialIndexPtr index = file->GetIndexer()->LoadIndex(index_file.string());
						HIDE_LOCK(_mutex);
						_partialIndexes.insert(std::make_pair(id, index));
						return;
					}
					catch (const std::exception& ex)
					{ s_logger.Warning() << "Cannot load " << file << " index: " << boost::diagnostic_information(ex); }
				}
			}
			catch (const std::exception& ex)
			{ s_logger.Warning() << "Cannot read " << file << " metainfo: " << boost::diagnostic_information(ex); }
		}

		s_logger.Info() << "Building " << file << " index";
		IPartialIndexPtr index = file->GetIndexer()->BuildIndex();
		{
			HIDE_LOCK(_mutex);
			_partialIndexes.insert(std::make_pair(id, index));
		}

		s_logger.Debug() << "Saving " << file << " index to " << index_file;
		boost::filesystem::create_directories(index_file.parent_path());
		index->Save(index_file.string());
		PartialIndexMetaInfo(file->GetModificationTime()).SaveToFile(meta_file);
	}


	void Indexer::OnFileRemoved(const IFilePtr& file)
	{
		IIndexableIdPtr id = file->GetIndexableId();
		HIDE_LOCK(_mutex);
		_partialIndexes.erase(id);
	}

}
