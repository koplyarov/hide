#ifndef HIDE_UTILS_ILOGGERSINK_H
#define HIDE_UTILS_ILOGGERSINK_H


#include <iostream>

#include <hide/utils/LoggerMessage.h>
#include <hide/utils/Utils.h>


namespace hide
{

	struct ILoggerSink
	{
		virtual ~ILoggerSink() { }

		virtual void PrintMessage(const LoggerMessage& msg) { } // Swig cannot make directors for abstract classes
	};
	HIDE_DECLARE_PTR(ILoggerSink);
	//HIDE_DECLARE_ARRAY(ILoggerSinkPtr);

}

#endif
