#include <hide/lang_plugins/cpp/CppCTagsIndexer.h>

#include <fstream>
#include <future>
#include <set>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/regex.hpp>
#include <boost/scope_exit.hpp>

#include <hide/lang_plugins/CTagsInvoker.h>
#include <hide/utils/FileSystemUtils.h>
#include <hide/utils/ReadBufferLinesListener.h>


namespace hide
{

	const std::string CppCTagsIndexer::s_tempDirectory(".hide/tmp");

	HIDE_NAMED_LOGGER(CppCTagsIndexer);

	CppCTagsIndexer::CppCTagsIndexer(const std::string& filename)
		: _filename(filename)
	{ }


	static std::set<std::string> GetFileDefines(const std::string& filename)
	{
		boost::regex re(R"(\s*#\s*define\s+([A-Za-z_][A-Za-z0-9_]*).*)");

		std::set<std::string> result;
		std::ifstream src_file(filename);
		std::string line;
		while (std::getline(src_file, line))
		{
			boost::smatch m;
			if (!boost::regex_match(line, m, re))
				continue;
			result.insert(m[1]);
		}

		return result;
	}


	static StringArray GetStdIncludePaths()
	{
		StringArray params = { "-v", "-x", "c++", "-E", "-" };
		ExecutablePtr gcc = std::make_shared<Executable>("g++", params);

		boost::optional<int> ret_code;
		gcc->AddListener(std::make_shared<FuncExecutableListener>(
			[&](int retCode) { ret_code = retCode; }
		));

		StringArray result;
		bool include_paths_section = false;
		std::promise<void> stderr_closed;
		gcc->GetStderr()->AddListener(std::make_shared<ReadBufferLinesListener>(
				[&](const std::string& s)
				{
					if (s == "End of search list.")
						include_paths_section = false;
					if (include_paths_section)
						result.push_back(boost::algorithm::trim_copy(s));
						//result.push_back(s);
					if (s == "#include <...> search starts here:")
						include_paths_section = true;
				},
				[&]() { stderr_closed.set_value(); }
		));

		stderr_closed.get_future().wait();
		gcc.reset();

		return result;
	}


	/*
	 * cpp -xc++ $BUILTIN_INCLUDES $PROJECT_INCLUDES -dM -o - srcfile | filter_defines > tmpfile.pp
	 * grep -v "^#\s*include" srcfile | cpp -w -include tmpfile.pp -o tmpfile.cpp
	 */
	IPartialIndexPtr CppCTagsIndexer::BuildIndex()
	{
		using namespace boost::filesystem;

		std::set<std::string> file_defines = GetFileDefines(_filename);

		StringArray std_include_paths = GetStdIncludePaths();
		if (std_include_paths.empty())
			s_logger.Warning() << "Empty default include paths!";

		path preprocessor_result = path(s_tempDirectory) / path(_filename + ".cppIndexer");
		boost::filesystem::create_directories(preprocessor_result.parent_path());
		BOOST_SCOPE_EXIT_ALL(&) {
			RemoveFileAndParentDirectories(preprocessor_result, s_tempDirectory);
		};

		std::promise<void> stderr_closed;
		boost::optional<int> ret_code;

		StringArray preprocessor_params = { "-xc++", "-std=c++11", "-I.", "-dM", "-o", preprocessor_result.string(), _filename };
		boost::transform(std_include_paths, std::back_inserter(preprocessor_params), [](const std::string& s) { return "-I" + s; });

		ExecutablePtr preprocessor = std::make_shared<Executable>("cpp", preprocessor_params);
		preprocessor->AddListener(std::make_shared<FuncExecutableListener>(
			[&](int retCode) { ret_code = retCode; }
		));
		preprocessor->GetStderr()->AddListener(std::make_shared<ReadBufferLinesListener>(
				[&](const std::string& s) { s_logger.Warning() << "g++ stderr: " << s; },
				[&]() { stderr_closed.set_value(); }
		));

		stderr_closed.get_future().wait();
		preprocessor.reset();

		HIDE_CHECK(*ret_code == 0, std::runtime_error(StringBuilder() % "Preprocessor failed, ret code: " % *ret_code));
		BOOST_THROW_EXCEPTION(std::runtime_error("Not implemented!"));

		IIndexEntryPtrArray entries;

		StringArray scope_fields = { "class", "struct", "namespace" };
		std::map<std::string, IndexEntryKind> kinds_map = { { "class", IndexEntryKind::Type }, { "struct", IndexEntryKind::Type }, { "member", IndexEntryKind::Variable }, { "function", IndexEntryKind::Function } };

		CTagsInvoker({ "-f-", "--excmd=number", "--sort=no", "--fields=+aimSztK", preprocessor_result.string() },
				[&](const std::string& name, int line, const CTagsOutputParser::FieldsMap& fields)
				{
					std::string scope;
					for (const auto& scope_field : scope_fields)
					{
						auto f_it = fields.find(scope_field);
						if (f_it != fields.end())
						{
							scope = f_it->second;
							break;
						}
					}

					std::string kind_str = fields.at("kind");
					auto k_it = kinds_map.find(fields.at("kind"));
					IndexEntryKind kind = k_it != kinds_map.end() ? k_it->second : IndexEntryKind();

					//entries.push_back(std::make_shared<CTagsIndexEntry>(name, scope, kind, Location(_filename, line, 1)));
				}
			);

		//return std::make_shared<CTagsPartialIndex>(entries);
		BOOST_THROW_EXCEPTION(std::runtime_error("Not implemented!"));
	}


	IPartialIndexPtr CppCTagsIndexer::LoadIndex(const std::string& filename)
	{
		BOOST_THROW_EXCEPTION(std::runtime_error("Not implemented!"));
	}

}
