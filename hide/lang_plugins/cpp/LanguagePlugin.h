#ifndef HIDE_LANG_PLUGINS_CPP_LANGUAGEPLUGIN_H
#define HIDE_LANG_PLUGINS_CPP_LANGUAGEPLUGIN_H


#include <hide/ILanguagePlugin.h>


namespace hide {
namespace cpp
{

	class LanguagePlugin : public virtual ILanguagePlugin
	{
	private:
		/* data */

	public:
		LanguagePlugin();

		virtual std::string GetLanguageName() const;
	};

}}

#endif
