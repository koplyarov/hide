#ifndef HIDE_UTILS_EXECUTABLE_H
#define HIDE_UTILS_EXECUTABLE_H

#include <functional>
#include <thread>

#include <boost/optional.hpp>

#if HIDE_PLATFORM_POSIX
#	include <sys/types.h>
#endif

#include <hide/utils/IReadBuffer.h>
#include <hide/utils/ListenersHolder.h>
#include <hide/utils/NamedLogger.h>
#include <hide/utils/Utils.h>


namespace hide
{

	struct IExecutableListener
	{
		virtual ~IExecutableListener() { }

		virtual void OnFinished(int retCode) { }
	};
	HIDE_DECLARE_PTR(IExecutableListener);


	class FuncExecutableListener : public IExecutableListener
	{
		typedef std::function<void(int)>	HandlerFunc;

	private:
		HandlerFunc		_handler;

	public:
		FuncExecutableListener(const HandlerFunc& handler) : _handler(handler) { }
		virtual void OnFinished(int retCode) { _handler(retCode); }
	};


	class Executable : public ListenersHolder<IExecutableListener>
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

		void Interrupt();

		IReadBufferPtr GetStdout() const { return _stdout; }
		IReadBufferPtr GetStderr() const { return _stderr; }

	protected:
		virtual void PopulateState(const IExecutableListenerPtr& listener) const;

	private:
		void ThreadFunc();
	};
	HIDE_DECLARE_PTR(Executable);

}

#endif
