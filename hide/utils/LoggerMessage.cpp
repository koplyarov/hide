#include <hide/utils/LoggerMessage.h>


namespace hide
{

	std::string LoggerMessage::ToString() const
	{
		std::string loglevel_str = "[" + _logLevel.ToString() + "] ";
		int whitespaces_count = sizeof("[Warning] ") - 1 - loglevel_str.size();
		if (whitespaces_count > 0)
			loglevel_str.append(whitespaces_count, ' ');

		std::string threadname_str = "{" + _threadName + "} ";
		whitespaces_count = 24 - threadname_str.size();
		if (whitespaces_count > 0)
			threadname_str.append(whitespaces_count, ' ');

		return StringBuilder() % loglevel_str % threadname_str % " [" % _loggerName % "] " % _text;
	}

}
