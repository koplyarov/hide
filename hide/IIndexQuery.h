#ifndef HIDE_IINDEXQUERY_H
#define HIDE_IINDEXQUERY_H


#include <string>

#include <hide/IPartialIndex.h>
#include <hide/Location.h>
#include <hide/utils/Utils.h>


namespace hide
{

	class IndexQueryEntry
	{
	private:
		std::string		_name;
		Location		_location;
		IndexEntryKind	_kind;

	public:
		IndexQueryEntry(const std::string& name, const Location& location, IndexEntryKind kind)
			: _name(name), _location(location), _kind(kind)
		{ }

		std::string GetName() const		{ return _name; }
		Location GetLocation() const	{ return _location; }
		IndexEntryKind GetKind() const	{ return _kind; }
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
