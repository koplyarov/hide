#ifndef HIDE_UTILS_IREADBUFFER_H
#define HIDE_UTILS_IREADBUFFER_H


#include <hide/utils/Utils.h>


namespace hide
{

	struct IReadBuffer;

	struct IReadBufferListener
	{
		virtual ~IReadBufferListener() { }

		virtual void OnBufferChanged(const IReadBuffer& buf) { }
	};
	HIDE_DECLARE_PTR(IReadBufferListener);

	struct IReadBuffer
	{
		virtual ~IReadBuffer() { }

		virtual void AddListener(const IReadBufferListenerPtr& listener) = 0;
		virtual void RemoveListener(const IReadBufferListenerPtr& listener) = 0;

		virtual ByteArray Read(int64_t ofs) const = 0; // Not very effective, but good for swig
	};
	HIDE_DECLARE_PTR(IReadBuffer);

}

#endif
