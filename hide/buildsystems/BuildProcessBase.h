#ifndef HIDE_BUILDSYSTEMS_BUILDPROCESSBASE_H
#define HIDE_BUILDSYSTEMS_BUILDPROCESSBASE_H


#include <deque>
#include <mutex>
#include <set>

#include <boost/optional.hpp>

#include <hide/IBuildSystem.h>
#include <hide/utils/ListenersHolder.h>
#include <hide/utils/Utils.h>


namespace hide
{

	class BuildProcessBase : public ListenersHolder<IBuildProcessListener, IBuildProcess>
	{
		typedef std::deque<BuildLogLine>														LogLines;
		typedef std::set<IBuildProcessListenerPtr, std::owner_less<IBuildProcessListenerPtr> >	ListenersSet;

	protected:
		boost::optional<bool>	_succeeded;
		LogLines				_lines;

	protected:
		void ReportLine(const BuildLogLine& line);
		void ReportFinished(bool succeeded);

		virtual void PopulateState(const IBuildProcessListenerPtr& listener) const;
	};

}

#endif
