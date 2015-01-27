#include <hide/lang_plugins/cpp/CompileCommandsJsonCompilationInfo.h>


namespace hide
{

	HIDE_NAMED_LOGGER(CompileCommandsJsonCompilationInfo);

	CompileCommandsJsonCompilationInfo::CompileCommandsJsonCompilationInfo(const std::string& jsonDatabase)
		: _jsonDatabase(jsonDatabase)
	{ }


	StringArray CompileCommandsJsonCompilationInfo::GetOptions(const boost::filesystem::path& file)
	{
		return { "-DHIDE_PLATFORM_POSIX=1", "-Dhide_EXPORTS", "-g", "-fPIC", "-I/usr/include/python2.7", "-I/usr/include/x86_64-linux-gnu/python2.7", "-I/usr/lib/llvm-3.5/include", "-I.", "-std=c++11", "-Wall", "-include", "hide/utils/ValgrindSupport.h", "-o", "CMakeFiles/hide.dir/hide/Buffer.cpp.o", "-c", "/home/koplyarov/work/hide/hide/Buffer.cpp" };
	}

}
