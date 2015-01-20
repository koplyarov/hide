#ifndef HIDE_UTILS_EXECUTABLE_H
#define HIDE_UTILS_EXECUTABLE_H

#include <functional>
#include <thread>

#include <boost/optional.hpp>

#if HIDE_PLATFORM_POSIX
#	include <sys/types.h>
#endif

#include <hide/utils/IPipeReadEndHandler.h>
#include <hide/utils/IPipeWriteEnd.h>
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
		HIDE_NONCOPYABLE(Executable);

		class PipeWriteEnd;
		HIDE_DECLARE_PTR(PipeWriteEnd);

		class PipeReadEnd;
		HIDE_DECLARE_PTR(PipeReadEnd);

	private:
		static NamedLogger			s_logger;

#if HIDE_PLATFORM_POSIX
		pid_t						_pid;
#endif

		boost::optional<int>		_retCode;
		IPipeWriteEndPtr			_stdin;
		PipeReadEndPtr				_stdout;
		PipeReadEndPtr				_stderr;
		std::thread					_thread;

	public:
		Executable(const std::string& executable, const StringArray& parameters, const IPipeReadEndHandlerPtr& stdoutHandler, const IPipeReadEndHandlerPtr& stderrHandler);
		Executable(const std::string& executable, const StringArray& parameters, const IPipeReadEndHandlerPtr& stdoutHandler, const NamedLogger& stderrLogger);
		~Executable();

		void Interrupt();

		IPipeWriteEndPtr GetStdin() const { return _stdin; }

	protected:
		virtual void PopulateState(const IExecutableListenerPtr& listener) const;

	private:
		void ThreadFunc();
	};
	HIDE_DECLARE_PTR(Executable);

}

#endif
