#ifndef HIDE_ILANGPLUGIN_H
#define HIDE_ILANGPLUGIN_H


#include <string>

#include <hide/Utils.h>


namespace hide
{

	struct ILanguagePlugin
	{
		virtual ~ILanguagePlugin() { }

		virtual std::string GetLanguageName() const = 0;
	};
	HIDE_DECLARE_PTR(ILanguagePlugin);

}

#endif
