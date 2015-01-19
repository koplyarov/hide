#ifndef HIDE_UTILS_IWRITEBUFFER_H
#define HIDE_UTILS_IWRITEBUFFER_H


#include <hide/utils/Utils.h>


namespace hide
{

	// TODO: refactor
	struct IWriteBuffer
	{
		virtual ~IWriteBuffer() { }

		virtual void Write(const ByteArray& data) = 0;
		virtual void Close() = 0;
	};
	HIDE_DECLARE_PTR(IWriteBuffer);

}

#endif
