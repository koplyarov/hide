#ifndef HIDE_LANG_PLUGINS_CPP_COMPILECOMMANDSJSONCOMPILATIONINFO_H
#define HIDE_LANG_PLUGINS_CPP_COMPILECOMMANDSJSONCOMPILATIONINFO_H


#include <hide/lang_plugins/cpp/ICppCompilationInfo.h>
#include <hide/utils/NamedLogger.h>


namespace hide
{

	class CompileCommandsJsonCompilationInfo : public ICppCompilationInfo
	{
	private:
		static NamedLogger		s_logger;
		std::string				_jsonDatabase;

	public:
		CompileCommandsJsonCompilationInfo(const std::string& jsonDatabase);

		virtual StringArray GetOptions(const boost::filesystem::path& file);
	};

}

#endif
