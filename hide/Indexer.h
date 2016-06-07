#ifndef HIDE_INDEXER_H
#define HIDE_INDEXER_H


#include <hide/IIndexQuery.h>
#include <hide/IIndexable.h>
#include <hide/ProjectFiles.h>
#include <hide/utils/Comparers.h>
#include <hide/utils/Diff.h>
#include <hide/utils/NamedLogger.h>
#include <hide/utils/rethread.h>

#include <boost/filesystem.hpp>

#include <condition_variable>
#include <mutex>
#include <queue>


namespace hide
{

	struct IIndexerListener
	{
		virtual ~IIndexerListener() { }

		virtual void OnIndexChanged(const Diff<IIndexEntryPtr>& diff) { HIDE_PURE_VIRTUAL_CALL(); }
	};
	HIDE_DECLARE_PTR(IIndexerListener);


	class Indexer : private ListenersHolder<IIndexerListener>
	{
		HIDE_NONCOPYABLE(Indexer);

		typedef ListenersHolder<IIndexerListener> ListenersHolderBase;

		struct EventType
		{
			HIDE_ENUM_VALUES(IndexableAdded, IndexableRemoved, IndexableModified, RemoveOutdatedFiles);
			HIDE_ENUM_CLASS(EventType);
		};

		struct Event
		{
			EventType			Type;
			IIndexablePtr		Indexable;

			HIDE_DECLARE_MEMBERS("type", &Indexer::Event::Type, "indexable", &Indexer::Event::Indexable);
		};

		class IndexQueryInfoBase;
		HIDE_DECLARE_PTR(IndexQueryInfoBase);

		class GenericIndexQueryInfo;
		class FastIndexQueryInfo;
		class IndexQuery;

		struct PartialIndexInfo;
		HIDE_DECLARE_PTR(PartialIndexInfo);

		typedef std::queue<IndexQueryInfoBasePtr>						QueriesQueue;
		typedef std::queue<Event>										EventQueue;
		typedef std::map<IIndexableIdPtr, PartialIndexInfoPtr, Less>	PartialIndexes;
		typedef std::multimap<std::string, IIndexEntryPtr>				StringToIndexEntry;

		class FilesListener;
		HIDE_DECLARE_PTR(FilesListener);

	private:
		static NamedLogger						s_logger;
		static const boost::filesystem::path	s_indexDirectory;
		mutable std::mutex						_mutex;
		mutable std::condition_variable			_condVar;
		bool									_working;
		QueriesQueue						_queries;
		EventQueue							_events;
		ProjectFilesPtr						_files;
		FilesListenerPtr					_filesListener;
		StringToIndexEntry					_symbolNameToSymbol;
		StringToIndexEntry					_fullSymbolNameToSymbol;
		PartialIndexes						_partialIndexes;
		thread								_thread;

	public:
		Indexer(const ProjectFilesPtr& files);
		~Indexer();

		IIndexQueryPtr QuerySymbolsBySubstring(const std::string& str);
		IIndexQueryPtr QuerySymbolsByName(const std::string& symbolName);

		virtual void AddListener(const IIndexerListenerPtr& listener)		{ ListenersHolderBase::AddListener(listener); }
		virtual void RemoveListener(const IIndexerListenerPtr& listener)	{ ListenersHolderBase::RemoveListener(listener); }

	protected:
		virtual void PopulateState(const IIndexerListenerPtr& listener) const;

	private:
		void ThreadFunc();

		IIndexQueryPtr DoQuerySymbols(const IndexQueryInfoBasePtr& queryInfo);

		void ProcessIndexableEvent(const Event& e);
		void RemoveOutdatedFiles();

		static boost::filesystem::path GetIndexFilePath(const IIndexablePtr& indexable);
		static boost::filesystem::path GetMetaFilePath(const IIndexablePtr& indexable);
	};
	HIDE_DECLARE_PTR(Indexer);

}

#endif
