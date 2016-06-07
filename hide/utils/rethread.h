#ifndef HIDE_UTILS_RETHREAD_H
#define HIDE_UTILS_RETHREAD_H


#include <rethread/cancellation_token.hpp>
#include <rethread/thread.hpp>


namespace hide
{

	using thread = rethread::thread;

	using standalone_cancellation_token = rethread::standalone_cancellation_token;
	using dummy_cancellation_token = rethread::dummy_cancellation_token;
	using cancellation_token = rethread::cancellation_token;

	using cancellation_guard = rethread::cancellation_guard;

	using cancellation_handler = rethread::cancellation_token;
}

#endif
