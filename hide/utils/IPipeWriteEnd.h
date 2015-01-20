#ifndef HIDE_UTILS_IPIPEWRITEEND_H
#define HIDE_UTILS_IPIPEWRITEEND_H


#include <hide/utils/Utils.h>


namespace hide
{

	struct IPipeWriteEnd
	{
		virtual ~IPipeWriteEnd() { }

		virtual void Write(const ByteArray& data) = 0;
		virtual void Close() = 0;
	};
	HIDE_DECLARE_PTR(IPipeWriteEnd);

}

#endif
