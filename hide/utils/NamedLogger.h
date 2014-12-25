#ifndef HIDE_UTILS_NAMEDLOGGER_H
#define HIDE_UTILS_NAMEDLOGGER_H


#include <sstream>

#include <hide/utils/Logger.h>
#include <hide/utils/StringBuilder.h>
#include <hide/utils/Utils.h>


namespace hide
{

#define HIDE_NAMED_LOGGER(...) NamedLogger __VA_ARGS__::s_logger(#__VA_ARGS__);

	class NamedLogger
	{
	public:
		class StreamAccessProxy
		{
			class Impl
			{
				friend class StreamAccessProxy;

			private:
				const NamedLogger*	_namedLogger;
				LogLevel			_logLevel;
				StringBuilder		_stringBuilder;

			public:
				Impl(const NamedLogger* namedLogger, LogLevel logLevel)
					: _namedLogger(namedLogger), _logLevel(logLevel)
				{ }

				~Impl()
				{ Logger::Log(LoggerMessage(_namedLogger->_name, _logLevel, _stringBuilder)); }
			};
			HIDE_DECLARE_PTR(Impl);

		private:
			ImplPtr		_impl;

		public:
			StreamAccessProxy(const NamedLogger* namedLogger, LogLevel logLevel)
			{
				if (logLevel.GetRaw() >= Logger::GetLogLevel().GetRaw())
					_impl.reset(new Impl(namedLogger, logLevel));
			}

			template < typename T >
			StreamAccessProxy& operator << (const T& val)
			{
				if (_impl)
					_impl->_stringBuilder % val;
				return *this;
			}
		};

	private:
		std::string		_name;

	public:
		NamedLogger(const std::string& name)
			: _name(name)
		{ }

		StreamAccessProxy Debug() const		{ return StreamAccessProxy(this, LogLevel::Debug); }
		StreamAccessProxy Info() const		{ return StreamAccessProxy(this, LogLevel::Info); }
		StreamAccessProxy Warning() const	{ return StreamAccessProxy(this, LogLevel::Warning); }
		StreamAccessProxy Error() const		{ return StreamAccessProxy(this, LogLevel::Error); }
	};

}

#endif
