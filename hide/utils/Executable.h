#ifndef HIDE_UTILS_EXECUTABLE_H
#define HIDE_UTILS_EXECUTABLE_H


#include <boost/optional.hpp>

#include <hide/utils/Logger.h>
#include <hide/utils/Utils.h>


namespace hide
{

	class Executable;
	HIDE_DECLARE_PTR(Executable);

	// TODO: design an asynchronous interface
	class Executable
	{
	private:
		static NamedLogger			s_logger;
		boost::optional<int>		_retCode;

	private:
		Executable(const std::string& executable, const StringArray& parameters);

	public:
		bool Succeeded() const	{ return *_retCode == 0; }

		static ExecutablePtr ExecuteSync(const std::string& executable, const StringArray& parameters);
	};

}

#endif
