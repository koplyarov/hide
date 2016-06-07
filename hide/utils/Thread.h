#ifndef HIDE_UTILS_THREAD_H
#define HIDE_UTILS_THREAD_H


#include <hide/utils/rethread.h>

#include <string>
#include <thread>


namespace hide
{

	std::string GetCurrentThreadName();
	void SetCurrentThreadName(const std::string& threadName);

	thread MakeThread(const std::string& threadName, const std::function<void(const cancellation_token&)>& threadFunc);

}

#endif
