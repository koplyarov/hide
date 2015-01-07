#ifndef HIDE_IPARTIALINDEXER_H
#define HIDE_IPARTIALINDEXER_H


#include <hide/IPartialIndex.h>
#include <hide/utils/Utils.h>


namespace hide
{

	struct IPartialIndexer
	{
		virtual ~IPartialIndexer() { }

		virtual IPartialIndexPtr BuildIndex() { HIDE_PURE_VIRTUAL_CALL(); }
		virtual IPartialIndexPtr LoadIndex(const std::string& filename) { HIDE_PURE_VIRTUAL_CALL(); }
	};
	HIDE_DECLARE_PTR(IPartialIndexer);

}

#endif
