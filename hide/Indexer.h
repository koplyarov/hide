#ifndef HIDE_INDEXER_H
#define HIDE_INDEXER_H


#include <condition_variable>
#include <mutex>
#include <queue>

#include <boost/filesystem.hpp>

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
			HIDE_ENUM_VALUES(IndexableAdded, IndexableRemoved, RemoveOutdatedFiles);
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
		static NamedLogger					s_logger;
		static const std::string			s_indexDirectory;
		mutable std::mutex					_mutex;
		mutable std::condition_variable		_condVar;
		bool								_working;
		QueriesQueue						_queries;
		EventQueue							_events;
		ProjectFilesPtr						_files;
		FilesListenerPtr					_filesListener;
		StringToIndexEntry					_symbolNameToSymbol;
		StringToIndexEntry					_fullSymbolNameToSymbol;
		PartialIndexes						_partialIndexes;
		std::thread							_thread;

	public:
		Indexer(const ProjectFilesPtr& files);
		~Indexer();

		IIndexQueryPtr QuerySymbolsBySubstring(const std::string& str);
		IIndexQueryPtr QuerySymbolsByName(const std::string& symbolName);

	private:
		void ThreadFunc();

		IIndexQueryPtr DoQuerySymbols(const IndexQueryInfoBasePtr& queryInfo);

		void AddIndexable(const IIndexablePtr& indexable);
		void RemoveIndexable(const IIndexablePtr& indexable);
		void RemoveOutdatedFiles();

		void AddPartialIndex(const IIndexableIdPtr& indexableId, const IPartialIndexPtr& index, const boost::filesystem::path& indexFilePath, const boost::filesystem::path& metaFilePath);
		void RemovePartialIndex(PartialIndexes::iterator indexableIt);

		static void DoRemoveFile(const boost::filesystem::path& filepath);
		static boost::filesystem::path GetIndexFilePath(const IIndexablePtr& indexable);
		static boost::filesystem::path GetMetaFilePath(const IIndexablePtr& indexable);
	};
	HIDE_DECLARE_PTR(Indexer);

}

#endif
