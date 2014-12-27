#include <hide/buildsystems/BuildProcessBase.h>


namespace hide
{

	void BuildProcessBase::ReportLine(const BuildLogLine& line)
	{
		HIDE_LOCK(GetMutex());
		_lines.push_back(line);
		InvokeListeners(std::bind(&IBuildProcessListener::OnLine, std::placeholders::_1, std::cref(line)));
	}


	void BuildProcessBase::ReportFinished(BuildStatus status)
	{
		HIDE_LOCK(GetMutex());
		_status = status;
		InvokeListeners(std::bind(&IBuildProcessListener::OnFinished, std::placeholders::_1, *_status));
	}


	void BuildProcessBase::PopulateState(const IBuildProcessListenerPtr& listener) const
	{
		for (auto l : _lines)
			listener->OnLine(l);
		if (_status)
			listener->OnFinished(*_status);
	}

}
