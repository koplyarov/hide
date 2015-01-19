#ifndef HIDE_LANG_PLUGINS_CPP_CPPCTAGSINDEXER_H
#define HIDE_LANG_PLUGINS_CPP_CPPCTAGSINDEXER_H


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
	};

}

#endif
