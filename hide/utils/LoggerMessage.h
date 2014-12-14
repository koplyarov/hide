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

		LogLevel()
			: _val(Debug)
		{ }

		std::string ToString() const
		{
			switch (GetRaw())
			{
			case LogLevel::Debug:	return "Debug";
			case LogLevel::Info:	return "Info";
			case LogLevel::Warning:	return "Warning";
			case LogLevel::Error:	return "Error";
			}
		}

		HIDE_DECLARE_SWIG_TO_STRING_WRAPPER();
	};


	class LoggerMessage
	{
	private:
		std::string		_loggerName;
		LogLevel		_logLevel;
		std::string		_text;

	public:
		LoggerMessage(const std::string& loggerName, LogLevel logLevel, const std::string& text)
			: _loggerName(loggerName), _logLevel(logLevel), _text(text)
		{ }

		std::string GetLoggerName() const	{ return _loggerName; }
		LogLevel GetLogLevel() const		{ return _logLevel; }
		std::string GetText() const 		{ return _text; }

		std::string ToString() const
		{
			std::string loglevel_str = "[" + _logLevel.ToString() + "]";
			int whitespaces_count = sizeof("[Warning]") - 1 - loglevel_str.size();
			if (whitespaces_count > 0)
				loglevel_str.append(whitespaces_count, ' ');
			return StringBuilder() % loglevel_str % " [" % _loggerName % "] " % _text;
		}

		HIDE_DECLARE_SWIG_TO_STRING_WRAPPER();
	};

}

#endif
