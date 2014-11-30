#ifndef HIDE_UTILS_LOGGER_H
#define HIDE_UTILS_LOGGER_H


#include <sstream>

#include <hide/utils/Utils.h>


namespace hide
{

#define HIDE_NAMED_LOGGER(Class_) NamedLogger Class_::s_logger(#Class_);

	struct LogLevel
	{
		HIDE_ENUM_VALUES(Debug, Info, Warning, Error);
		HIDE_ENUM_CLASS(LogLevel);
	};


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
				std::stringstream	_stream;

			public:
				Impl(const NamedLogger* namedLogger, LogLevel logLevel)
					: _namedLogger(namedLogger), _logLevel(logLevel)
				{ }

				~Impl()
				{
					std::string loglevel_str;
					switch (_logLevel.GetRaw())
					{
					case LogLevel::Debug:	loglevel_str = "[Debug]  ";	break;
					case LogLevel::Info:	loglevel_str = "[Info]   ";	break;
					case LogLevel::Warning:	loglevel_str = "[Warning]";	break;
					case LogLevel::Error:	loglevel_str = "[Error]  ";	break;
					}
					std::cerr << loglevel_str << " [" << _namedLogger->_name << "] " << _stream.str() << std::endl;
				}
			};
			HIDE_DECLARE_PTR(Impl);

		private:
			ImplPtr		_impl;

		public:
			StreamAccessProxy(const NamedLogger* namedLogger, LogLevel logLevel)
				: _impl(new Impl(namedLogger, logLevel))
			{ }

			template < typename T >
			StreamAccessProxy& operator << (const T& val)
			{ _impl->_stream << val; return *this; }
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
