#ifndef HIDE_UTILS_IPIPEREADENDHANDLER_H
#define HIDE_UTILS_IPIPEREADENDHANDLER_H


#include <hide/utils/Utils.h>


namespace hide
{

	struct IPipeReadEndHandler
	{
		virtual ~IPipeReadEndHandler() { }

		virtual void OnData(const ByteArray& data) { HIDE_PURE_VIRTUAL_CALL(); }
		virtual void OnEndOfData() { HIDE_PURE_VIRTUAL_CALL(); }
	};
	HIDE_DECLARE_PTR(IPipeReadEndHandler);

}

#endif
