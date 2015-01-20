#ifndef HIDE_LANG_PLUGINS_CTAGSINVOKER_H
#define HIDE_LANG_PLUGINS_CTAGSINVOKER_H


#include <functional>
#include <future>
#include <map>
#include <string>
#include <vector>

#include <boost/regex.hpp>

#include <hide/utils/Executable.h>
#include <hide/utils/NamedLogger.h>
#include <hide/utils/Utils.h>


namespace hide
{

	class CTagsOutputParser
	{
	public:
		typedef std::map<std::string, std::string> FieldsMap;
		typedef std::function<void(const std::string& name, int line, const FieldsMap& fields)>		TagHandlerFunc;

	private:
		static NamedLogger	s_logger;
		boost::regex		_ctagsLineRegex;
		TagHandlerFunc		_tagHandler;

	public:
		CTagsOutputParser(const TagHandlerFunc& tagHandler);

		void ProcessLine(const std::string& str);
	};
	HIDE_DECLARE_PTR(CTagsOutputParser);


	class CTagsInvoker
	{
	private:
		static NamedLogger		s_logger;
		ExecutablePtr			_executable;

	public:
		CTagsInvoker(const StringArray& parameters, const CTagsOutputParser::TagHandlerFunc& tagHandler);
		~CTagsInvoker();
	};
	HIDE_DECLARE_PTR(CTagsInvoker);

}

#endif
