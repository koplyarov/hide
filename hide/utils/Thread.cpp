#include <hide/utils/Thread.h>

#include <hide/utils/NamedLogger.h>
#include <hide/utils/StringBuilder.h>
#include <hide/utils/rethread.h>


namespace hide
{

	struct ThreadUtils
	{
		static NamedLogger s_logger;

		static std::string& GetThreadNameRef()
		{
			thread_local std::string thread_name(StringBuilder() % std::hex % "0x" % std::this_thread::get_id());
			return thread_name;
		}

		static void ThreadFuncWrapper(const std::string& threadName, const std::function<void(const cancellation_token&)>& threadFunc, const cancellation_token& t)
		{
			GetThreadNameRef() = threadName;

			try
			{ threadFunc(t); }
			catch (const std::exception& ex)
			{ s_logger.Error() << "Uncaught exception in thread function: " << ex; }
		}
	};

	HIDE_NAMED_LOGGER(ThreadUtils);




	std::string GetCurrentThreadName()
	{ return ThreadUtils::GetThreadNameRef(); }


	void SetCurrentThreadName(const std::string& threadName)
	{ ThreadUtils::GetThreadNameRef() = threadName; }


	thread MakeThread(const std::string& threadName, const std::function<void(const cancellation_token&)>& threadFunc)
	{ return thread(std::bind(&ThreadUtils::ThreadFuncWrapper, threadName, threadFunc, std::placeholders::_1)); }

}
