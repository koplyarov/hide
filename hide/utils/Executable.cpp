#include <hide/utils/Executable.h>

#include <sstream>

#include <boost/regex.hpp>


namespace hide
{

	HIDE_NAMED_LOGGER(Executable);

	Executable::Executable(const std::string& executable, const StringArray& parameters) // TODO: implement async invokation
	{
#if HIDE_PLATFORM_POSIX
		using namespace boost;
		std::stringstream shell_cmd;
		shell_cmd << executable;
		for (auto p : parameters)
		{
			shell_cmd << " '" << regex_replace(p, regex("(')"), "\\$1") << "'"; // TODO: escape anything else?
		}

		s_logger.Debug() << "Command: " << shell_cmd.str();

		_retCode = system(shell_cmd.str().c_str());
#else
#	error Executable::ExecuteSync is not implemented
#endif
	}


	ExecutablePtr Executable::ExecuteSync(const std::string& executable, const StringArray& parameters)
	{
		ExecutablePtr result(new Executable(executable, parameters));
		// TODO: wait for the execution to complete
		return result;
	}

}
