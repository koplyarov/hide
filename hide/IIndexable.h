#ifndef HIDE_IINDEXABLE_H
#define HIDE_IINDEXABLE_H


#include <hide/IPartialIndexer.h>
#include <hide/utils/IComparable.h>
#include <hide/utils/Utils.h>


namespace hide
{

	struct IIndexableId : public virtual IComparable
	{
		virtual ~IIndexableId() { }

		virtual std::string ToString() const { HIDE_PURE_VIRTUAL_CALL(); }
	};
	HIDE_DECLARE_PTR(IIndexableId);


	struct IIndexable
	{
		virtual ~IIndexable() { }

		virtual IIndexableIdPtr GetIndexableId() const { HIDE_PURE_VIRTUAL_CALL(); }
		virtual Time GetModificationTime() const { HIDE_PURE_VIRTUAL_CALL(); }
		virtual IPartialIndexerPtr GetIndexer() { HIDE_PURE_VIRTUAL_CALL(); }

		virtual std::string ToString() const { return GetIndexableId()->ToString(); }
	};
	HIDE_DECLARE_PTR(IIndexable);

}

#endif
