#ifndef HIDE_IINDEXQUERY_H
#define HIDE_IINDEXQUERY_H


#include <string>

#include <hide/Location.h>
#include <hide/utils/Utils.h>


namespace hide
{

	class IndexQueryEntry
	{
	private:
		std::string		_name;
		Location		_location;

	public:
		IndexQueryEntry(const std::string& name, const Location& location)
			: _name(name), _location(location)
		{ }

		std::string GetName() const		{ return _name; }
		Location GetLocation() const	{ return _location; }
	};
	HIDE_DECLARE_ARRAY(IndexQueryEntry);


	struct IIndexQueryListener
	{
		virtual ~IIndexQueryListener() { }

		virtual void OnFinished() { HIDE_PURE_VIRTUAL_CALL(); }
		virtual void OnEntry(const IndexQueryEntry& entry) { HIDE_PURE_VIRTUAL_CALL(); }
	};
	HIDE_DECLARE_PTR(IIndexQueryListener);


	struct IIndexQuery
	{
		virtual ~IIndexQuery() { }

		virtual void AddListener(const IIndexQueryListenerPtr& listener) { HIDE_PURE_VIRTUAL_CALL(); }
		virtual void RemoveListener(const IIndexQueryListenerPtr& listener) { HIDE_PURE_VIRTUAL_CALL(); }
	};
	HIDE_DECLARE_PTR(IIndexQuery);

}

#endif
