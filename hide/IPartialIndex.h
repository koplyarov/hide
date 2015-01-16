#ifndef HIDE_IPARTIALINDEX_H
#define HIDE_IPARTIALINDEX_H


#include <string>

#include <hide/Location.h>
#include <hide/utils/IComparable.h>
#include <hide/utils/StringBuilder.h>
#include <hide/utils/Utils.h>


namespace hide
{

	struct IndexEntryKind
	{
		HIDE_ENUM_VALUES(NamedConstant, Variable, Function, Type);
		HIDE_ENUM_CLASS(IndexEntryKind);

		HIDE_DECLARE_SWIG_TO_STRING_WRAPPER();
	};


	struct IIndexEntry : public virtual IComparable
	{
		virtual ~IIndexEntry() { }

		virtual std::string GetName() const { HIDE_PURE_VIRTUAL_CALL(); }
		virtual std::string GetFullName() const { HIDE_PURE_VIRTUAL_CALL(); }
		virtual IndexEntryKind GetKind() const { HIDE_PURE_VIRTUAL_CALL(); }
		virtual Location GetLocation() const { HIDE_PURE_VIRTUAL_CALL(); }

		virtual std::string ToString() const { return GetName(); }
	};
	HIDE_DECLARE_PTR(IIndexEntry);
	HIDE_DECLARE_ARRAY(IIndexEntryPtr);


	struct IPartialIndex
	{
		virtual ~IPartialIndex() { }

		virtual void Save(const std::string& filename) { HIDE_PURE_VIRTUAL_CALL(); }

		virtual IIndexEntryPtrArray GetEntries() { HIDE_PURE_VIRTUAL_CALL(); }
	};
	HIDE_DECLARE_PTR(IPartialIndex);

}

#endif
