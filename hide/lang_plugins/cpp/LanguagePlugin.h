#ifndef HIDE_LANG_PLUGINS_CPP_LANGUAGEPLUGIN_H
#define HIDE_LANG_PLUGINS_CPP_LANGUAGEPLUGIN_H


#include <hide/ILanguagePlugin.h>
#include <hide/lang_plugins/cpp/ICppCompilationInfo.h>


namespace hide {
namespace cpp
{

	class LanguagePlugin : public virtual ILanguagePlugin
	{
	private:
		ICppCompilationInfoPtr		_compilationInfo;

	public:
		LanguagePlugin();

		virtual std::string GetLanguageName() const;

		virtual IFilePtr ProbeFile(const std::string& filename) const;
	};

}}

#endif
