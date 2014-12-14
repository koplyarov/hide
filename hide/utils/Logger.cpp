#include <hide/utils/Logger.h>

#include <boost/exception/diagnostic_information.hpp>
#include <boost/thread/mutex.hpp>


namespace hide
{

	HIDE_DECLARE_ARRAY(ILoggerSinkPtr);

	static boost::mutex			g_loggerMutex;
	static LogLevel				g_logLevel = LogLevel::Info;
	static ILoggerSinkPtrArray	g_loggerSinks; // TODO: reimplement


	LogLevel Logger::GetLogLevel()
	{
		boost::mutex::scoped_lock l(g_loggerMutex);
		return g_logLevel;
	}


	void Logger::Log(const LoggerMessage& msg)
	{
		boost::mutex::scoped_lock l(g_loggerMutex);

		if (msg.GetLogLevel().GetRaw() < g_logLevel)
			return;

		for (auto s : g_loggerSinks)
		{
			try
			{
				if (s)
					s->PrintMessage(msg);
			}
			catch (const std::exception& ex)
			{ std::cerr << "Exception in logger sink: " << boost::diagnostic_information(ex) << std::endl; }
		}

		//std::cerr << "C++ " << msg.ToString() << std::endl;
	}


	void Logger::SetLogLevel(LogLevel logLevel)
	{
		boost::mutex::scoped_lock l(g_loggerMutex);
		g_logLevel = logLevel;
	}


	void Logger::RegisterSink(const ILoggerSinkPtr& sink)
	{
		boost::mutex::scoped_lock l(g_loggerMutex);
		g_loggerSinks.push_back(sink);
	}

}
