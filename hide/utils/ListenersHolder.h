#ifndef HIDE_UTILS_LISTENERSHOLDER_H
#define HIDE_UTILS_LISTENERSHOLDER_H


#include <functional>
#include <mutex>
#include <set>

#include <boost/mpl/empty_base.hpp>

#include <hide/utils/NamedLogger.h>
#include <hide/utils/Utils.h>


namespace hide
{

	template < typename ListenerType_, typename Interface_ = boost::mpl::empty_base >
	class ListenersHolder : public virtual Interface_
	{
		typedef ListenerType_												ListenerType;
		HIDE_DECLARE_PTR(ListenerType);

		typedef std::set<ListenerTypePtr, std::owner_less<ListenerTypePtr> >	ListenersSet;

	private:
		static NamedLogger				s_logger;
		ListenersSet					_listeners;
		mutable std::recursive_mutex	_mutex;

	public:
		virtual void AddListener(const ListenerTypePtr& listener)
		{
			HIDE_LOCK(_mutex);
			_listeners.insert(listener);
			try
			{ PopulateState(listener); }
			catch (const std::exception& ex)
			{ s_logger.Error() << "Uncaught exception in PopulateState for listener: " << ex; }
		}

		virtual void RemoveListener(const ListenerTypePtr& listener)
		{
			HIDE_LOCK(_mutex);
			_listeners.erase(listener);
		}

	protected:
		std::recursive_mutex& GetMutex() const
		{ return _mutex; }

		virtual void PopulateState(const ListenerTypePtr& listener) const = 0;

		void InvokeListeners(const std::function<void(const ListenerTypePtr&)>& f) const
		{
			HIDE_LOCK(_mutex);
			for (const auto& l : _listeners)
			{
				try
				{ f(l); }
				catch (const std::exception& ex)
				{ s_logger.Error() << "Uncaught exception in listener: " << ex; }
			}
		}
	};

	template < typename ListenerType_, typename Interface_ >
	HIDE_NAMED_LOGGER(ListenersHolder<ListenerType_, Interface_>);

}

#endif
