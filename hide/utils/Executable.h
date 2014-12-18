#ifndef HIDE_UTILS_EXECUTABLE_H
#define HIDE_UTILS_EXECUTABLE_H

#include <thread>

#include <boost/optional.hpp>

#if HIDE_PLATFORM_POSIX
#	include <sys/types.h>
#endif

#include <hide/utils/IReadBuffer.h>
#include <hide/utils/NamedLogger.h>
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

#if HIDE_PLATFORM_POSIX
		pid_t						_pid;
#endif

		boost::optional<int>		_retCode;
		IReadBufferPtr				_stdout;
		IReadBufferPtr				_stderr;
		std::thread					_thread;

	public:
		Executable(const std::string& executable, const StringArray& parameters);
		~Executable();

		IReadBufferPtr GetStdout() const { return _stdout; }
		IReadBufferPtr GetStderr() const { return _stderr; }

		bool Succeeded() const	{ return *_retCode == 0; }

	private:
		void ThreadFunc();
	};

}

#endif
