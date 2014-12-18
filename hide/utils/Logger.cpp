#include <hide/utils/Logger.h>

#include <mutex>
#include <set>

#include <boost/exception/diagnostic_information.hpp>
#include <boost/thread/mutex.hpp>


namespace hide
{

	typedef std::set<ILoggerSinkPtr, std::owner_less<ILoggerSinkPtr> >	LoggerSinksSet;

	static std::recursive_mutex			g_loggerMutex;
	static LogLevel						g_logLevel = LogLevel::Info;
	static LoggerSinksSet				g_loggerSinks; // TODO: reimplement


	LogLevel Logger::GetLogLevel()
	{
		HIDE_LOCK(g_loggerMutex);
		return g_logLevel;
	}


	void Logger::Log(const LoggerMessage& msg)
	{
		HIDE_LOCK(g_loggerMutex);

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
		HIDE_LOCK(g_loggerMutex);
		g_logLevel = logLevel;
	}


	void Logger::AddSink(const ILoggerSinkPtr& sink)
	{
		HIDE_LOCK(g_loggerMutex);
		g_loggerSinks.insert(sink);
	}


	void Logger::RemoveSink(const ILoggerSinkPtr& sink)
	{
		HIDE_LOCK(g_loggerMutex);
		g_loggerSinks.erase(sink);
	}

}
