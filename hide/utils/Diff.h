#ifndef HIDE_UTILS_DIFF_H
#define HIDE_UTILS_DIFF_H


#include <hide/utils/Utils.h>


namespace hide
{

	template < typename T >
	class Diff
	{
	public:
		typedef T			Element;
		HIDE_DECLARE_ARRAY(Element);

	private:
		ElementArray		_added;
		ElementArray		_removed;

	public:
		Diff(const ElementArray& added, const ElementArray& removed) : _added(added), _removed(removed) { }
#ifndef SWIG
		Diff(ElementArray&& added, ElementArray&& removed) : _added(added), _removed(removed) { }
#endif

		ElementArray GetAdded() const	{ return _added; }
		ElementArray GetRemoved() const	{ return _removed; }
	};

}

#endif
