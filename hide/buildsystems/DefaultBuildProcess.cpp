#include <hide/buildsystems/DefaultBuildProcess.h>

#include <hide/utils/ReadBufferLinesListener.h>


namespace hide
{

	HIDE_NAMED_LOGGER(DefaultBuildProcess);


	DefaultBuildProcess::DefaultBuildProcess(const std::string& executable, const StringArray& parameters)
	{
		_executable = std::make_shared<Executable>(executable, parameters);
		_executable->GetStdout()->AddListener(std::make_shared<ReadBufferLinesListener>(std::bind(&DefaultBuildProcess::ParseLine, this, std::placeholders::_1)));
		s_logger.Debug() << "Created";
	}


	DefaultBuildProcess::~DefaultBuildProcess()
	{
		s_logger.Debug() << "Destroying";
		_executable.reset();
	}


	void DefaultBuildProcess::ParseLine(const std::string& str)
	{
		s_logger.Debug() << "Stdout: " << str;
		// TODO: parse errors/warnings
		ReportLine(BuildLogLine(str, nullptr));
	}


	void DefaultBuildProcess::OnFinished()
	{
		ReportFinished(_executable->Succeeded());
	}

}
