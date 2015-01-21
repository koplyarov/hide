#ifndef HIDE_LANG_PLUGINS_CPP_CPPCTAGSINDEXER_H
#define HIDE_LANG_PLUGINS_CPP_CPPCTAGSINDEXER_H


#include <set>

#include <boost/filesystem.hpp>

#include <hide/IPartialIndexer.h>
#include <hide/utils/NamedLogger.h>


namespace hide
{

	class CppCTagsIndexer : public virtual IPartialIndexer
	{
	private:
		static NamedLogger			s_logger;
		static const std::string	s_tempDirectory;
		std::string					_filename;

	public:
		CppCTagsIndexer(const std::string& filename);

		virtual IPartialIndexPtr BuildIndex();
		virtual IPartialIndexPtr LoadIndex(const std::string& filename);

	private:
		std::set<std::string> GetFileDefines(const std::string& filename);
		StringArray GetStdIncludePaths();
		void GenerateIncludeFile(const boost::filesystem::path& dst);
		void PreprocessFile(const boost::filesystem::path& includeFile, const boost::filesystem::path& dst);
	};

}

#endif
