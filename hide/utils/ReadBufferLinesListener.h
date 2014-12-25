#ifndef HIDE_UTILS_READBUFFERLINESLISTENER_H
#define HIDE_UTILS_READBUFFERLINESLISTENER_H


#include <functional>
#include <string>

#include <stdint.h>

#include <hide/utils/IReadBuffer.h>
#include <hide/utils/NamedLogger.h>
#include <hide/utils/Utils.h>


namespace hide
{

	class ReadBufferLinesListener : public virtual IReadBufferListener
	{
		typedef std::function<void(const std::string&)>		BufferCallbackFunc;
		typedef std::function<void()>						EndOfDataCallbackFunc;

	private:
		static NamedLogger		s_logger;
		BufferCallbackFunc		_bufferChangedCallback;
		EndOfDataCallbackFunc	_endOfDataCallback;
		int64_t					_ofs;
		std::string				_accumStr;

	public:
		ReadBufferLinesListener(const BufferCallbackFunc& bufferChangedCallback, const EndOfDataCallbackFunc& endOfDataCallback);

		virtual void OnBufferChanged(const IReadBuffer& buf);
		virtual void OnEndOfData();
	};

}

#endif
