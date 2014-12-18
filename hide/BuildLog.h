#ifndef HIDE_BUILDLOG_H
#define HIDE_BUILDLOG_H


#include <deque>
#include <mutex>
#include <set>

#include <boost/optional.hpp>

#include <hide/Location.h>
#include <hide/utils/Utils.h>


namespace hide
{

	struct BuildIssueType
	{
		HIDE_ENUM_VALUES(Warning, Error);
		HIDE_ENUM_CLASS(BuildIssueType);
	};


	class BuildIssue
	{
	private:
		Location			_location;
		BuildIssueType		_type;
		std::string			_text;

	public:
		BuildIssue(const Location& location, BuildIssueType type, const std::string& text)
			: _location(location), _type(type), _text(text)
		{ }

		Location GetLocation() const	{ return _location; }
		BuildIssueType GetType() const	{ return _type; }
		std::string GetText() const		{ return _text; }
	};
	HIDE_DECLARE_PTR(BuildIssue);


	class BuildLogLine
	{
	public:
		std::string			_text;
		BuildIssuePtr		_issue;

	public:
		BuildLogLine(const std::string& text, const BuildIssuePtr& issue)
			: _text(text), _issue(issue)
		{ }

		std::string GetText() const		{ return _text; }
		BuildIssuePtr GetIssue() const	{ return _issue; }
	};


	class BuildLog;

	struct IBuildLogListener
	{
		virtual ~IBuildLogListener() { }

		virtual void OnLine(const BuildLogLine& line) { }
		virtual void OnFinished(bool succeeded) { }
	};
	HIDE_DECLARE_PTR(IBuildLogListener);


	class BuildLog
	{
		typedef std::deque<BuildLogLine>												LogLines;
		typedef std::set<IBuildLogListenerPtr, std::owner_less<IBuildLogListenerPtr> >	ListenersSet;

	protected:
		std::recursive_mutex	_mutex;
		boost::optional<bool>	_succeeded;
		LogLines				_lines;
		ListenersSet			_listeners;

	public:
		void AddListener(const IBuildLogListenerPtr& listener);
		void RemoveListener(const IBuildLogListenerPtr& listener);
	};
	HIDE_DECLARE_PTR(BuildLog);


	class BuildLogControl : public BuildLog
	{
	public:
		void ReportLine(const BuildLogLine& line);
		void ReportFinished(bool succeeded);
	};
	HIDE_DECLARE_PTR(BuildLogControl);

}

#endif
