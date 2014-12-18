#ifndef HIDE_UTILS_READBUFFERBASE_H
#define HIDE_UTILS_READBUFFERBASE_H


#include <mutex>
#include <set>

#include <hide/utils/IReadBuffer.h>


namespace hide
{

	class ReadBufferBase : public virtual IReadBuffer
	{
		typedef std::set<IReadBufferListenerPtr, std::owner_less<IReadBufferListenerPtr> >	ListenersMap;

	protected:
		mutable std::recursive_mutex	_mutex;
		ListenersMap					_listeners;

	public:
		virtual void AddListener(const IReadBufferListenerPtr& listener)
		{
			HIDE_LOCK(_mutex);
			_listeners.insert(listener);
		}

		virtual void RemoveListener(const IReadBufferListenerPtr& listener)
		{
			HIDE_LOCK(_mutex);
			_listeners.erase(listener);
		}
	};

}

#endif
