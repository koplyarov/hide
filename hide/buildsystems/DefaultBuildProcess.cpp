#include <hide/buildsystems/DefaultBuildProcess.h>


namespace hide
{

	class DefaultBuildProcess::StdoutListener : public virtual IReadBufferListener
	{
	private:
		static NamedLogger		s_logger;
		BuildLogControlPtr		_buildLogControl;
		int64_t					_ofs;
		std::string				_accumStr;

	public:
		StdoutListener(const BuildLogControlPtr& buildLogControl)
			: _buildLogControl(buildLogControl), _ofs(0)
		{ }

		virtual void OnBufferChanged(const IReadBuffer& buf)
		{
			ByteArray data = buf.Read(_ofs);

			auto new_line_it = std::find(data.begin(), data.end(), '\n');
			std::copy(data.begin(), new_line_it, std::back_inserter(_accumStr));
			if (new_line_it != data.end())
			{
				ReportLine(_accumStr);
				_accumStr.clear();
				std::copy(new_line_it + 1, data.end(), std::back_inserter(_accumStr));
			}

			_ofs += data.size();
		}

	private:
		void ReportLine(const std::string& str)
		{
			// TODO: parse errors/warnings
			_buildLogControl->ReportLine(BuildLogLine(str, nullptr));
		}
	};
	HIDE_NAMED_LOGGER(DefaultBuildProcess::StdoutListener);


	HIDE_NAMED_LOGGER(DefaultBuildProcess);

	DefaultBuildProcess::DefaultBuildProcess(const std::string& executable, const StringArray& parameters)
		: _executable(executable, parameters), _buildLogControl(std::make_shared<BuildLogControl>())
	{
		_executable.GetStdout()->AddListener(std::make_shared<StdoutListener>(_buildLogControl));
		s_logger.Debug() << "Created";
	}


	DefaultBuildProcess::~DefaultBuildProcess()
	{
		s_logger.Debug() << "Destroying";
	}


	void DefaultBuildProcess::OnFinished()
	{
		_buildLogControl->ReportFinished(_executable.Succeeded());
	}

}
