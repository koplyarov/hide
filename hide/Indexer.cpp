#include <hide/Indexer.h>

#include <hide/utils/ListenersHolder.h>


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
			_thread = std::thread(std::bind(&TestIndexQuery::ThreadFunc, this));
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


	void Indexer::OnFileAdded(const IFilePtr& file)
	{
		IIndexableIdPtr id = file->GetIndexableId();
		IPartialIndexPtr index = file->GetIndexer()->BuildIndex();
		HIDE_LOCK(_mutex);
		_partialIndexes.insert(std::make_pair(id, index));
	}

}
