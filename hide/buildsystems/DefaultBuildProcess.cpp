#include <hide/buildsystems/DefaultBuildProcess.h>

#include <boost/regex.hpp>

#include <hide/utils/ReadBufferLinesListener.h>


namespace hide
{

	HIDE_NAMED_LOGGER(DefaultBuildProcess);


	class DefaultBuildProcess::ExecutableListener : public virtual IExecutableListener
	{
	private:
		DefaultBuildProcess*	_inst;

	public:
		ExecutableListener(DefaultBuildProcess* inst)
			: _inst(inst)
		{ }

		virtual void OnFinished(int retCode)
		{ _inst->SetRetCode(retCode); }
	};


	DefaultBuildProcess::DefaultBuildProcess(const std::string& executable, const StringArray& parameters)
		: _stdoutClosed(false), _interrupted(false)
	{
		_executable = std::make_shared<Executable>(executable, parameters);
		_executable->GetStdout()->AddListener(std::make_shared<ReadBufferLinesListener>(
				std::bind(&DefaultBuildProcess::ParseLine, this, std::placeholders::_1),
				std::bind(&DefaultBuildProcess::PipeClosedHandler, this, std::ref(_stdoutClosed))
			));
		_executable->AddListener(std::make_shared<ExecutableListener>(this));
		s_logger.Info() << "Created";
	}


	DefaultBuildProcess::~DefaultBuildProcess()
	{
		s_logger.Info() << "Destroying";
		_executable.reset();
	}


	void DefaultBuildProcess::Interrupt()
	{
		s_logger.Info() << "Interrupt()";
		_executable->Interrupt();
		HIDE_LOCK(_mutex);
		_interrupted = true;
	}


	void DefaultBuildProcess::ParseLine(const std::string& str)
	{
		using namespace boost;

		s_logger.Debug() << "Stdout: " << str;

		BuildIssuePtr issue;

		// TODO: Implement customizable parsing
		regex re(R"(^(.+):(\d+):(\d+): ([^:]+):\s+(.+)$)");
		smatch m;
		if (regex_match(str, m, re))
		{
			BuildIssueType issue_type;
			if (m[4] == "note")
				issue_type = BuildIssueType::Note;
			if (m[4] == "warning")
				issue_type = BuildIssueType::Warning;
			else if (m[4] == "error")
				issue_type = BuildIssueType::Error;
			issue = std::make_shared<BuildIssue>(Location(m[1], std::stoll(m[2]), std::stoll(m[3])), issue_type, m[5]);
		}

		ReportLine(BuildLogLine(str, issue));
	}


	void DefaultBuildProcess::PipeClosedHandler(bool& flag)
	{
		s_logger.Info() << "DefaultBuildProcess::PipeClosedHandler(bool& flag)";
		HIDE_LOCK(_mutex);
		flag = true;
		TryReportFinished();
	}


	void DefaultBuildProcess::SetRetCode(int retCode)
	{
		s_logger.Info() << "DefaultBuildProcess::SetRetCode(" << retCode << ")";
		HIDE_LOCK(_mutex);
		_retCode = retCode;
		TryReportFinished();
	}


	void DefaultBuildProcess::TryReportFinished()
	{
		if (_stdoutClosed && _retCode)
		{
			BuildStatus status = _interrupted ? BuildStatus::Interrupted : (*_retCode == 0 ? BuildStatus::Succeeded : BuildStatus::Failed);
			s_logger.Info() << "Build " << status;
			ReportFinished(status);
		}
	}

}
