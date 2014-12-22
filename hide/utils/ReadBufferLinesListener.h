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
		typedef std::function<void(const std::string&)>		CallbackFunc;

	private:
		static NamedLogger		s_logger;
		CallbackFunc			_callback;
		int64_t					_ofs;
		std::string				_accumStr;

	public:
		ReadBufferLinesListener(const CallbackFunc& callback);

		virtual void OnBufferChanged(const IReadBuffer& buf);
	};

}

#endif
