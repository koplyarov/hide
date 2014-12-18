#include <hide/BuildLog.h>


namespace hide
{

	void BuildLog::AddListener(const IBuildLogListenerPtr& listener)
	{
		HIDE_LOCK(_mutex);
		_listeners.insert(listener);

		for (auto l : _lines)
			listener->OnLine(l);
		if (_succeeded)
			listener->OnFinished(*_succeeded);
	}


	void BuildLog::RemoveListener(const IBuildLogListenerPtr& listener)
	{
		HIDE_LOCK(_mutex);
		_listeners.erase(listener);
	}


	void BuildLogControl::ReportLine(const BuildLogLine& line)
	{
		HIDE_LOCK(_mutex);
		_lines.push_back(line);
		for (auto l : _listeners)
			l->OnLine(line);
	}


	void BuildLogControl::ReportFinished(bool succeeded)
	{
		HIDE_LOCK(_mutex);
		_succeeded = succeeded;
		for (auto l : _listeners)
			l->OnFinished(*_succeeded);
	}

}
