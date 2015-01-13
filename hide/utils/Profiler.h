#ifndef HIDE_UTILS_PROFILER_H
#define HIDE_UTILS_PROFILER_H


#include <chrono>
#include <stdint.h>


namespace hide
{

	template < typename TimeT = std::chrono::microseconds, typename ClockT = std::chrono::high_resolution_clock >
	class Profiler
	{
		typedef std::chrono::time_point<ClockT>		TimePoint;

	private:
		TimePoint		_start;

	public:
		Profiler()
		{ _start = ClockT::now(); }

		decltype(TimePoint() - TimePoint()) Reset()
		{
			TimePoint end = ClockT::now();
			auto delta = end - _start;
			_start = end;
			return delta;
		}
	};

}

#endif
