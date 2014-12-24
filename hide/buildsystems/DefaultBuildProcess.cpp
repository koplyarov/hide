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
		{
			s_logger.Info() << "Build " << (retCode == 0 ? "succeeded" : "failed");
			_inst->ReportFinished(retCode == 0);
		}
	};


	DefaultBuildProcess::DefaultBuildProcess(const std::string& executable, const StringArray& parameters)
	{
		_executable = std::make_shared<Executable>(executable, parameters);
		_executable->GetStdout()->AddListener(std::make_shared<ReadBufferLinesListener>(std::bind(&DefaultBuildProcess::ParseLine, this, std::placeholders::_1)));
		_executable->AddListener(std::make_shared<ExecutableListener>(this));
		// TODO: fix inconsistent order of reporting lines and finished flag
		s_logger.Info() << "Created";
	}


	DefaultBuildProcess::~DefaultBuildProcess()
	{
		s_logger.Info() << "Destroying";
		_executable.reset();
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

}
