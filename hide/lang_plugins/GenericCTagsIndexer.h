#ifndef HIDE_LANG_PLUGINS_CPP_CTAGSINDEXER_H
#define HIDE_LANG_PLUGINS_CPP_CTAGSINDEXER_H


#include <hide/IPartialIndexer.h>
#include <hide/utils/NamedLogger.h>


namespace hide
{

	class GenericCTagsIndexer : public virtual IPartialIndexer
	{
	private:
		static NamedLogger	s_logger;
		std::string			_filename;

	public:
		GenericCTagsIndexer(const std::string& filename);

		virtual IPartialIndexPtr BuildIndex();
		virtual IPartialIndexPtr LoadIndex(const std::string& filename);
	};
}

#endif
