#include <hide/Indexer.h>

#include <boost/filesystem.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <hide/utils/ListenersHolder.h>
#include <hide/utils/PTree.h>


namespace hide
{


	class Indexer::FilesListener : public virtual IProjectFilesListener
	{
	private:
		Indexer*		_inst;

	public:
		FilesListener(Indexer* inst) : _inst(inst) { }

		virtual void OnFileAdded(const IFilePtr& file)
		{ _inst->OnFileAdded(file); }

		virtual void OnFileRemoved(const IFilePtr& file)
		{ }
	};


	HIDE_NAMED_LOGGER(Indexer);

	Indexer::Indexer(const ProjectFilesPtr& files)
		: _files(files)
	{
		_filesListener = std::make_shared<FilesListener>(this);
		_files->AddListener(_filesListener);
	}


	Indexer::~Indexer()
	{
		_files->RemoveListener(_filesListener);
		_filesListener.reset();
	}


	class TestIndexQuery : public ListenersHolder<IIndexQueryListener, IIndexQuery>
	{
		typedef std::vector<IndexQueryEntry>					Entries;
		typedef std::function<bool(const IIndexEntryPtr&)>		CheckEntryFunc;

	private:
		const Indexer*	_indexer;
		CheckEntryFunc	_checkEntryFunc;
		std::thread		_thread;
		Entries			_entries;
		bool			_finished;

	public:
		TestIndexQuery(const Indexer* indexer, const CheckEntryFunc& checkEntryFunc)
			: _indexer(indexer), _checkEntryFunc(checkEntryFunc), _finished(false)
		{
			_thread = MakeThread("testIndeQuery", std::bind(&TestIndexQuery::ThreadFunc, this));
		}

		~TestIndexQuery()
		{
			_thread.join();
		}

	protected:
		virtual void PopulateState(const IIndexQueryListenerPtr& listener) const
		{
			for (auto e : _entries)
				listener->OnEntry(e);

			if (_finished)
				listener->OnFinished();
		}

	private:
		void ReportEntry(const IndexQueryEntry& entry)
		{
			HIDE_LOCK(GetMutex());
			_entries.push_back(entry);
			InvokeListeners(std::bind(&IIndexQueryListener::OnEntry, std::placeholders::_1, entry));
		}

		void ReportFinished()
		{
			HIDE_LOCK(GetMutex());
			if (_finished)
				return;
			_finished = true;
			InvokeListeners(std::bind(&IIndexQueryListener::OnFinished, std::placeholders::_1));
		}

		void ThreadFunc()
		{
			HIDE_LOCK(_indexer->_mutex);
			for (auto pi : _indexer->_partialIndexes)
				for (auto e : pi.second->GetEntries())
					if (_checkEntryFunc(e))
						ReportEntry(IndexQueryEntry(e->GetFullName(), e->GetLocation()));
			ReportFinished();
		}
	};


	IIndexQueryPtr Indexer::QuerySymbolsBySubstring(const std::string& str)
	{
		return std::make_shared<TestIndexQuery>(this, [str](const IIndexEntryPtr& e) { return e->GetName().find(str) != std::string::npos; }); // TODO: use ImplPtr
	}

	IIndexQueryPtr Indexer::QuerySymbolsByName(const std::string& symbolName)
	{
		return std::make_shared<TestIndexQuery>(this, [symbolName](const IIndexEntryPtr& e) { return e->GetName() == symbolName; }); // TODO: use ImplPtr
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
				boost::property_tree::ptree meta_root;
				std::ifstream f(meta_file.string());
				boost::property_tree::read_json(f, meta_root);
				PTreeReader wr(meta_root);
				PTreeReader(meta_root).Read("metaInfo", old_meta_info);

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

		boost::property_tree::ptree meta_root;
		PTreeWriter w(meta_root);
		w.Write("metaInfo", PartialIndexMetaInfo(file->GetModificationTime()));
		std::ofstream f(meta_file.string(), std::ios_base::trunc);
		boost::property_tree::write_json(f, meta_root, false);
	}

}
