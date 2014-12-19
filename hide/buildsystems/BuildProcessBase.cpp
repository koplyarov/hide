#include <hide/buildsystems/BuildProcessBase.h>


namespace hide
{

	void BuildProcessBase::AddListener(const IBuildProcessListenerPtr& listener)
	{
		HIDE_LOCK(_mutex);
		_listeners.insert(listener);

		for (auto l : _lines)
			listener->OnLine(l);
		if (_succeeded)
			listener->OnFinished(*_succeeded);
	}


	void BuildProcessBase::RemoveListener(const IBuildProcessListenerPtr& listener)
	{
		HIDE_LOCK(_mutex);
		_listeners.erase(listener);
	}


	void BuildProcessBase::ReportLine(const BuildLogLine& line)
	{
		HIDE_LOCK(_mutex);
		_lines.push_back(line);
		for (auto l : _listeners)
			l->OnLine(line);
	}


	void BuildProcessBase::ReportFinished(bool succeeded)
	{
		HIDE_LOCK(_mutex);
		_succeeded = succeeded;
		for (auto l : _listeners)
			l->OnFinished(*_succeeded);
	}

}
