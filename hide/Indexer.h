#ifndef HIDE_INDEXER_H
#define HIDE_INDEXER_H


#include <condition_variable>
#include <mutex>
#include <queue>

#include <boost/variant.hpp>

#include <hide/IIndexQuery.h>
#include <hide/IIndexable.h>
#include <hide/ProjectFiles.h>
#include <hide/utils/Comparers.h>
#include <hide/utils/NamedLogger.h>


namespace hide
{

	class Indexer
	{
		HIDE_NONCOPYABLE(Indexer);

		friend class TestIndexQuery;

		struct EventType
		{
			HIDE_ENUM_VALUES(IndexableAdded, IndexableRemoved);
			HIDE_ENUM_CLASS(EventType);
		};

		struct Event
		{
			EventType			Type;
			IIndexablePtr		Indexable;

			Event(EventType type, const IIndexablePtr indexable)
				: Type(type), Indexable(indexable)
			{ }
		};

		class IndexQueryInfo;
		HIDE_DECLARE_PTR(IndexQueryInfo);

		class IndexQuery;

		typedef std::queue<IndexQueryInfoPtr>							QueriesQueue;
		typedef std::queue<Event>										EventQueue;
		typedef std::map<IIndexableIdPtr, IPartialIndexPtr, Less>		PartialIndexes;

		class FilesListener;
		HIDE_DECLARE_PTR(FilesListener);

	private:
		static NamedLogger					s_logger;
		mutable std::mutex					_mutex;
		mutable std::condition_variable		_condVar;
		bool								_working;
		QueriesQueue						_queries;
		EventQueue							_events;
		ProjectFilesPtr						_files;
		FilesListenerPtr					_filesListener;
		PartialIndexes						_partialIndexes;
		std::thread							_thread;

	public:
		Indexer(const ProjectFilesPtr& files);
		~Indexer();

		IIndexQueryPtr QuerySymbolsBySubstring(const std::string& str);
		IIndexQueryPtr QuerySymbolsByName(const std::string& symbolName);

	private:
		void ThreadFunc();

		IIndexQueryPtr DoQuerySymbols(const std::function<bool(const IIndexEntryPtr&)>& checkEntryFunc);

		void OnFileAdded(const IFilePtr& file);
		void OnFileRemoved(const IFilePtr& file);
	};
	HIDE_DECLARE_PTR(Indexer);

}

#endif
