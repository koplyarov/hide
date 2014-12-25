#ifndef HIDE_UTILS_THREAD_H
#define HIDE_UTILS_THREAD_H


#include <string>
#include <thread>


namespace hide
{

	std::string GetCurrentThreadName();
	void SetCurrentThreadName(const std::string& threadName);

	std::thread MakeThread(const std::string& threadName, const std::function<void()>& threadFunc);

}

#endif
