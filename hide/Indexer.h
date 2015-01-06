#ifndef HIDE_INDEXER_H
#define HIDE_INDEXER_H


#include <mutex>

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

		typedef std::map<IIndexableIdPtr, IPartialIndexPtr, Less>		PartialIndexes;

		class FilesListener;
		HIDE_DECLARE_PTR(FilesListener);

	private:
		static NamedLogger				s_logger;
		mutable std::recursive_mutex	_mutex;
		ProjectFilesPtr					_files;
		FilesListenerPtr				_filesListener;
		PartialIndexes					_partialIndexes;

	public:
		Indexer(const ProjectFilesPtr& files);
		~Indexer();

		IIndexQueryPtr QuerySymbolsBySubstring(const std::string& str);

	private:
		void OnFileAdded(const IFilePtr& file);
	};
	HIDE_DECLARE_PTR(Indexer);

}

#endif
