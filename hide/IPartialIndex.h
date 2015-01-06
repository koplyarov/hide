#ifndef HIDE_IPARTIALINDEX_H
#define HIDE_IPARTIALINDEX_H


#include <string>

#include <hide/Location.h>
#include <hide/utils/Utils.h>


namespace hide
{

	struct IIndexEntry
	{
		virtual ~IIndexEntry() { }

		virtual std::string GetName() const { HIDE_PURE_VIRTUAL_CALL(); }
		virtual std::string GetFullName() const { HIDE_PURE_VIRTUAL_CALL(); }
		virtual Location GetLocation() const { HIDE_PURE_VIRTUAL_CALL(); }

		virtual std::string ToString() const { return GetName(); }
	};
	HIDE_DECLARE_PTR(IIndexEntry);
	HIDE_DECLARE_ARRAY(IIndexEntryPtr);


	struct IPartialIndex
	{
		virtual ~IPartialIndex() { }

		virtual Time GetModificationTime() { HIDE_PURE_VIRTUAL_CALL(); }
		virtual IIndexEntryPtrArray GetEntries() { HIDE_PURE_VIRTUAL_CALL(); }
	};
	HIDE_DECLARE_PTR(IPartialIndex);

}

#endif
