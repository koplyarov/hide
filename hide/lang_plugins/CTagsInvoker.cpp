#include <hide/lang_plugins/CTagsInvoker.h>

#include <hide/utils/ReadBufferLinesListener.h>


namespace hide
{

	HIDE_NAMED_LOGGER(CTagsOutputParser);

	CTagsOutputParser::CTagsOutputParser(const TagHandlerFunc& tagHandler)
		:	_ctagsLineRegex("^([^	]+)	([^	]+)	(\\d+);\"	(.*)$"),
			_tagHandler(tagHandler)
	{ }


	void CTagsOutputParser::ProcessLine(const std::string& str)
	{
		boost::smatch m;
		if (boost::regex_match(str, m, _ctagsLineRegex))
		{
			std::string name = m[1];
			int line = std::stoi(m[3]);
			FieldsMap fields;
			std::string field_to_parse;
			std::stringstream stream(m[4]);
			while (std::getline(stream, field_to_parse, '\t'))
			{
				size_t colon_pos = field_to_parse.find(':');
				if (colon_pos == std::string::npos)
				{
					s_logger.Warning() << "Could not parse ctags tag field: '" << field_to_parse << "'";
					continue;
				}
				fields.insert(std::make_pair(field_to_parse.substr(0, colon_pos), field_to_parse.substr(colon_pos + 1)));
			}

			_tagHandler(name, line, fields);
		}
		else
			s_logger.Warning() << "Could not parse ctags output line: '" << str << "'";
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	HIDE_NAMED_LOGGER(CTagsInvoker);

	CTagsInvoker::CTagsInvoker(const StringArray& parameters, const CTagsOutputParser::TagHandlerFunc& tagHandler)
	{
		auto output_parser = std::make_shared<CTagsOutputParser>(tagHandler);
		_executable = std::make_shared<Executable>("ctags", parameters);
		_executable->GetStdout()->AddListener(std::make_shared<ReadBufferLinesListener>(
				[output_parser](const std::string& s) { output_parser->ProcessLine(s); },
				[&]() { _stdoutClosed.set_value(); }
			));
		_executable->GetStderr()->AddListener(std::make_shared<ReadBufferLinesListener>(
				[output_parser](const std::string& s) { s_logger.Warning() << "ctags stderr: " << s; },
				[&]() { _stderrClosed.set_value(); }
			));
		_executable->GetStdin()->Close();
	}


	CTagsInvoker::~CTagsInvoker()
	{
		_stdoutClosed.get_future().wait();
		_stderrClosed.get_future().wait();
		_executable.reset();
	}

}
