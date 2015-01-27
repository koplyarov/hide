#ifndef HIDE_LANG_PLUGINS_CPP_FILE_H
#define HIDE_LANG_PLUGINS_CPP_FILE_H


#include <hide/IFile.h>
#include <hide/lang_plugins/cpp/ICppCompilationInfo.h>


namespace hide {
namespace cpp
{

	class File : public virtual IFile
	{
	private:
		std::string					_filename;
		ICppCompilationInfoPtr		_compilationInfo;

	public:
		File(const std::string& filename, const ICppCompilationInfoPtr& compilationInfo);

		virtual std::string GetFilename() const { return _filename; }

		virtual IIndexableIdPtr GetIndexableId() const;
		virtual Time GetModificationTime() const;
		virtual IPartialIndexerPtr GetIndexer();
	};

}}

#endif
