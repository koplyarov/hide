#include <iostream>

#include <hide/Project.h>
#include <hide/utils/ILoggerSink.h>

#include <boost/exception/diagnostic_information.hpp>

#include <chrono>
#include <thread>


class LoggerSink : public virtual hide::ILoggerSink
{
	virtual void PrintMessage(const hide::LoggerMessage& msg)
	{ printf("%s\n", msg.ToString().c_str()); }
};


int main()
{
	try
	{
		hide::SetCurrentThreadName("main");

		hide::ILoggerSinkPtr sink = std::make_shared<LoggerSink>();
		hide::Logger::AddSink(sink);
		hide::Logger::SetLogLevel(hide::LogLevel::Debug);

		hide::ProjectPtr p = hide::Project::CreateAuto({".*\\bCMakeFiles\\b.*", ".*\\.git\\b.*"});

		std::this_thread::sleep_for(std::chrono::seconds(45));

		hide::Logger::RemoveSink(sink);
	}
	catch (const std::exception& ex)
	{
		std::cerr << "test failed: " << boost::diagnostic_information(ex) << std::endl;
	}
}
