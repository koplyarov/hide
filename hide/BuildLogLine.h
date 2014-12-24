#ifndef HIDE_BUILDLOGLINE_H
#define HIDE_BUILDLOGLINE_H


#include <hide/Location.h>
#include <hide/utils/StringBuilder.h>
#include <hide/utils/Utils.h>


namespace hide
{

	struct BuildIssueType
	{
		HIDE_ENUM_VALUES(Unknown, Note, Warning, Error);
		HIDE_ENUM_CLASS(BuildIssueType);

		BuildIssueType()
			: _val(Unknown)
		{ }

		std::string ToString() const
		{
			switch (GetRaw())
			{
			case BuildIssueType::Unknown:	return "Unknown";
			case BuildIssueType::Note:		return "Note";
			case BuildIssueType::Warning:	return "Warning";
			case BuildIssueType::Error:		return "Error";
			}
		}

		HIDE_DECLARE_SWIG_TO_STRING_WRAPPER();
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

}

#endif
