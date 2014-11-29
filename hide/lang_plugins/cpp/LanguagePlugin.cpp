#include <hide/lang_plugins/cpp/LanguagePlugin.h>

#include <boost/filesystem.hpp>
#include <boost/range/algorithm.hpp>

#include <hide/lang_plugins/cpp/File.h>


namespace hide {
namespace cpp
{

	LanguagePlugin::LanguagePlugin()
	{
	}


	std::string LanguagePlugin::GetLanguageName() const
	{
		return "cpp";
	}


	IFilePtr LanguagePlugin::ProbeFile(const std::string& filename) const
	{
		using namespace boost;
		using namespace boost::filesystem;

		std::vector<std::string> extensions = { ".h", ".hpp", ".c", ".cpp" };
		if (find(extensions, path(filename).extension().string()) != extensions.end())
			return std::make_shared<File>(filename);
		return nullptr;
	}

}}
