#include <hide/Indexer.h>

#include <hide/utils/ListenersHolder.h>


namespace hide
{

	HIDE_NAMED_LOGGER(Indexer);

	Indexer::Indexer()
	{
	}


	Indexer::~Indexer()
	{
	}


	class TestIndexQuery : public ListenersHolder<IIndexQueryListener, IIndexQuery>
	{
		typedef std::vector<IndexQueryEntry>		Entries;

	private:
		std::thread		_thread;
		Entries			_entries;
		bool			_finished;

	public:
		TestIndexQuery()
			: _finished(false)
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
			ReportEntry(IndexQueryEntry("ZOMG LOL", Location()));
			std::this_thread::sleep_for(std::chrono::seconds(3));
			//std::this_thread::sleep_for(std::chrono::milliseconds(300));
			ReportEntry(IndexQueryEntry("AZAZA", Location()));
			ReportFinished();
		}
	};


	IIndexQueryPtr Indexer::QuerySymbolsBySubstring(const std::string& str)
	{
		return std::make_shared<TestIndexQuery>();
	}

}
