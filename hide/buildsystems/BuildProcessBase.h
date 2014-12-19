#ifndef HIDE_BUILDSYSTEMS_BUILDPROCESSBASE_H
#define HIDE_BUILDSYSTEMS_BUILDPROCESSBASE_H


#include <deque>
#include <mutex>
#include <set>

#include <boost/optional.hpp>

#include <hide/IBuildSystem.h>
#include <hide/utils/Utils.h>


namespace hide
{

	class BuildProcessBase : public IBuildProcess
	{
		typedef std::deque<BuildLogLine>														LogLines;
		typedef std::set<IBuildProcessListenerPtr, std::owner_less<IBuildProcessListenerPtr> >	ListenersSet;

	protected:
		std::recursive_mutex	_mutex;
		boost::optional<bool>	_succeeded;
		LogLines				_lines;
		ListenersSet			_listeners;

	public:
		void AddListener(const IBuildProcessListenerPtr& listener);
		void RemoveListener(const IBuildProcessListenerPtr& listener);

	protected:
		void ReportLine(const BuildLogLine& line);
		void ReportFinished(bool succeeded);
	};

}

#endif
