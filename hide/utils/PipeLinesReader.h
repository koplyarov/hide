#ifndef HIDE_UTILS_PIPELINESREADER_H
#define HIDE_UTILS_PIPELINESREADER_H


#include <functional>
#include <string>

#include <stdint.h>

#include <hide/utils/IPipeReadEndHandler.h>
#include <hide/utils/NamedLogger.h>
#include <hide/utils/Utils.h>


namespace hide
{

	class PipeLinesReader : public virtual IPipeReadEndHandler
	{
		typedef std::function<void(const std::string&)>		BufferCallbackFunc;

	private:
		static NamedLogger		s_logger;
		BufferCallbackFunc		_bufferChangedCallback;
		std::string				_accumStr;

	public:
		PipeLinesReader(const BufferCallbackFunc& bufferChangedCallback);

		virtual void OnData(const ByteArray& data);
		virtual void OnEndOfData();
	};

}

#endif
