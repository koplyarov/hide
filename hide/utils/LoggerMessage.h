#ifndef HIDE_UTILS_LOGGERMESSAGE_H
#define HIDE_UTILS_LOGGERMESSAGE_H


#include <string>

#include <hide/utils/StringBuilder.h>
#include <hide/utils/Utils.h>


namespace hide
{

	struct LogLevel
	{
		HIDE_ENUM_VALUES(Debug, Info, Warning, Error);
		HIDE_ENUM_CLASS(LogLevel);

		HIDE_DECLARE_SWIG_TO_STRING_WRAPPER();
	};


	class LoggerMessage
	{
	private:
		std::string		_threadName;
		std::string		_loggerName;
		LogLevel		_logLevel;
		std::string		_text;

	public:
		LoggerMessage(const std::string& threadName, const std::string& loggerName, LogLevel logLevel, const std::string& text)
			: _threadName(threadName), _loggerName(loggerName), _logLevel(logLevel), _text(text)
		{ }

		std::string GetThreadName() const	{ return _threadName; }
		std::string GetLoggerName() const	{ return _loggerName; }
		LogLevel GetLogLevel() const		{ return _logLevel; }
		std::string GetText() const 		{ return _text; }

		std::string ToString() const;

		HIDE_DECLARE_SWIG_TO_STRING_WRAPPER();
	};

}

#endif
