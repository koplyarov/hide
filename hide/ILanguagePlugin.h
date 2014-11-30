#ifndef HIDE_ILANGPLUGIN_H
#define HIDE_ILANGPLUGIN_H


#include <string>

#include <hide/IFile.h>
#include <hide/utils/Utils.h>


namespace hide
{

	struct ILanguagePlugin
	{
		virtual ~ILanguagePlugin() { }

		virtual std::string GetLanguageName() const = 0;

		virtual IFilePtr ProbeFile(const std::string& filename) const = 0;
	};
	HIDE_DECLARE_PTR(ILanguagePlugin);
	HIDE_DECLARE_ARRAY(ILanguagePluginPtr);

}

#endif
