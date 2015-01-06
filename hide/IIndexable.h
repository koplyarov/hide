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
	};
	HIDE_DECLARE_PTR(IIndexableId);


	struct IIndexable
	{
		virtual ~IIndexable() { }

		virtual IIndexableIdPtr GetIndexableId() { HIDE_PURE_VIRTUAL_CALL(); }
		virtual Time GetModificationTime() { HIDE_PURE_VIRTUAL_CALL(); }
		virtual IPartialIndexerPtr GetIndexer() { HIDE_PURE_VIRTUAL_CALL(); }
	};
	HIDE_DECLARE_PTR(IIndexable);

}

#endif
