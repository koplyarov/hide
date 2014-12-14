#ifndef HIDE_UTILS_LOGGER_H
#define HIDE_UTILS_LOGGER_H


#include <hide/utils/ILoggerSink.h>
#include <hide/utils/LoggerMessage.h>


namespace hide
{

	class Logger
	{
		friend class NamedLogger;

	private:
		static LogLevel GetLogLevel();
		static void Log(const LoggerMessage& msg);

	public:
		static void SetLogLevel(LogLevel logLevel);
		static void RegisterSink(const ILoggerSinkPtr& sink);
	};

}

#endif
