#include <hide/lang_plugins/cpp/CppCTagsIndexer.h>

#include <fstream>
#include <future>
#include <set>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/regex.hpp>
#include <boost/scope_exit.hpp>

#include <hide/lang_plugins/CTagsInvoker.h>
#include <hide/utils/Comparers.h>
#include <hide/utils/FileSystemUtils.h>
#include <hide/utils/PTree.h>
#include <hide/utils/ReadBufferLinesListener.h>


namespace hide
{

	const std::string CppCTagsIndexer::s_tempDirectory(".hide/tmp");

	HIDE_NAMED_LOGGER(CppCTagsIndexer);

	CppCTagsIndexer::CppCTagsIndexer(const std::string& filename)
		: _filename(filename)
	{ }


	class CppCTagsIndexEntry : public Comparable<CppCTagsIndexEntry>, public virtual IIndexEntry
	{
	public:
		struct SerializationProxy
		{
			std::string		Name;
			std::string		Scope;
			IndexEntryKind	Kind;
			int				Line;

			SerializationProxy() : Line(0) { }
			SerializationProxy(const CppCTagsIndexEntry& e) : Name(e._name), Scope(e._scope), Kind(e._kind), Line(e._location.GetLine()) { }

			CppCTagsIndexEntry ToEntry(const std::string& filename) const { return CppCTagsIndexEntry(Name, Scope, Kind, Location(filename, Line, 1)); }

			HIDE_DECLARE_MEMBERS("name", &HIDE_SELF_TYPE::Name, "scope", &HIDE_SELF_TYPE::Scope, "kind", &HIDE_SELF_TYPE::Kind, "line", &HIDE_SELF_TYPE::Line);
		};

	private:
		std::string		_name;
		std::string		_scope;
		IndexEntryKind	_kind;
		Location		_location;

	public:
		CppCTagsIndexEntry()
		{ }

		CppCTagsIndexEntry(const std::string& name, const std::string& scope, IndexEntryKind kind, const Location& location)
			: _name(name), _scope(scope), _kind(kind), _location(location)
		{ }

		virtual std::string GetName() const		{ return _name; }
		virtual std::string GetScope() const	{ return _scope; }
		virtual std::string GetFullName() const	{ return _scope.empty() ? _name : _scope + "::" + _name; }
		virtual IndexEntryKind GetKind() const	{ return _kind; }
		virtual Location GetLocation() const	{ return _location; }

		HIDE_DECLARE_MEMBERS("name", &CppCTagsIndexEntry::_name, "scope", &CppCTagsIndexEntry::_scope, "kind", &CppCTagsIndexEntry::_kind, "location", &CppCTagsIndexEntry::_location)

	protected:
		virtual int DoCompare(const CppCTagsIndexEntry& other) const
		{ return Cmp()(GetFullName(), other.GetFullName()); } // TODO: compare member lists
	};


	class CppCTagsPartialIndex : public virtual IPartialIndex
	{
	private:
		IIndexEntryPtrArray		_entries;

	public:
		CppCTagsPartialIndex(const IIndexEntryPtrArray& entries)
			: _entries(entries)
		{ }

		virtual void Save(const std::string& filename)
		{
			using namespace boost::property_tree;

			std::vector<CppCTagsIndexEntry::SerializationProxy> ctags_entries;
			std::transform(_entries.begin(), _entries.end(), std::back_inserter(ctags_entries), [](const IIndexEntryPtr& e) { return dynamic_cast<const CppCTagsIndexEntry&>(*e); });

			ptree root;
			PTreeWriter w(root);
			w.Write("entries", ctags_entries);

			std::ofstream f(filename, std::ios_base::trunc);
			write_json(f, root, false);
		}

		virtual IIndexEntryPtrArray GetEntries() { return _entries; }
	};


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

		gcc->GetStdin()->Close();

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
	 * cpp -xc++ $BUILTIN_INCLUDES $PROJECT_INCLUDES -dM -o - srcfile | filter_src_defines > tmpfile.pp
	 * grep -v "^#\s*include" srcfile | cpp -w -include tmpfile.pp -o tmpfile.cpp
	 */
	IPartialIndexPtr CppCTagsIndexer::BuildIndex()
	{
		using namespace boost::filesystem;

		std::set<std::string> file_defines = GetFileDefines(_filename);

		StringArray std_include_paths = GetStdIncludePaths();
		if (std_include_paths.empty())
			s_logger.Warning() << "Empty default include paths!";

		path file_to_include = path(s_tempDirectory) / path(_filename + ".cppIndexer.inc");
		BOOST_SCOPE_EXIT_ALL(&) {
			RemoveFileAndParentDirectories(file_to_include, s_tempDirectory);
		};

		path preprocessor_result = path(s_tempDirectory) / path(_filename + ".cppIndexer.cpp");
		boost::filesystem::create_directories(preprocessor_result.parent_path());
		BOOST_SCOPE_EXIT_ALL(&) {
			RemoveFileAndParentDirectories(preprocessor_result, s_tempDirectory);
		};

		{
			StringArray preprocessor_params = { "-xc++", "-std=c++11", "-DHIDE_PLATFORM_POSIX", "-I.", "-dM", "-o-", _filename };
			boost::transform(std_include_paths, std::back_inserter(preprocessor_params), [](const std::string& s) { return "-I" + s; });

			boost::optional<int> ret_code;
			ExecutablePtr preprocessor = std::make_shared<Executable>("cpp", preprocessor_params);

			preprocessor->GetStdin()->Close();

			preprocessor->AddListener(std::make_shared<FuncExecutableListener>(
				[&](int retCode) { ret_code = retCode; }
			));

			std::promise<void> stdout_closed;
			boost::regex define_re(R"(\s*#\s*define\s+([A-Za-z_][A-Za-z0-9_]*).*)");
			std::ofstream file_to_include_f(file_to_include.string(), std::ios::binary | std::ios::out);
			preprocessor->GetStdout()->AddListener(std::make_shared<ReadBufferLinesListener>(
					[&](const std::string& s)
					{
						boost::smatch m;
						if (!boost::regex_match(s, m, define_re))
						{
							s_logger.Warning() << "'" << s << "' is not a valid define!";
							return;
						}
						if (file_defines.find(m[1]) == file_defines.end())
							file_to_include_f << s << "\n";
					},
					[&]() { stdout_closed.set_value(); }
			));

			std::promise<void> stderr_closed;
			preprocessor->GetStderr()->AddListener(std::make_shared<ReadBufferLinesListener>(
					[&](const std::string& s) { s_logger.Warning() << "cpp stderr: " << s; },
					[&]() { stderr_closed.set_value(); }
			));

			stdout_closed.get_future().wait();
			stderr_closed.get_future().wait();
			preprocessor.reset();

			HIDE_CHECK(*ret_code == 0, std::runtime_error(StringBuilder() % "Preprocessor failed, ret code: " % *ret_code));
		}

		{
			StringArray preprocessor_params = { "-w", "-xc++", "-std=c++11", "-include", file_to_include.string(), "-o-", "-" };

			boost::optional<int> ret_code;
			ExecutablePtr preprocessor = std::make_shared<Executable>("cpp", preprocessor_params);
			preprocessor->AddListener(std::make_shared<FuncExecutableListener>(
				[&](int retCode) { ret_code = retCode; }
			));

			boost::regex pp_stuff_re(R"X(\s*#\s+(\d+)\s+"([^"]+)".*)X"); // " This comment fixes vim syntax highlight =)
			std::promise<void> stdout_closed;
			std::ofstream preprocessor_result_f(preprocessor_result.string(), std::ios::binary | std::ios::out);
			int line_num = 1;
			preprocessor->GetStdout()->AddListener(std::make_shared<ReadBufferLinesListener>(
					[&](const std::string& s)
					{
						boost::smatch m;
						if (!boost::regex_match(s, m, pp_stuff_re))
						{
							preprocessor_result_f << s << "\n";
							++line_num;
						}
						else if (m[2] == "<stdin>")
						{
							int set_next_line_num = std::stoi(m[1]);
							if (set_next_line_num < line_num)
								s_logger.Warning() << "Invalid line_num in preprocessor output!";
							else
								for (; line_num < set_next_line_num; ++line_num)
									preprocessor_result_f << "\n";
						}
					},
					[&]() { stdout_closed.set_value(); }
			));

			std::promise<void> stderr_closed;
			preprocessor->GetStderr()->AddListener(std::make_shared<ReadBufferLinesListener>(
					[&](const std::string& s) { s_logger.Warning() << "cpp stderr: " << s; },
					[&]() { stderr_closed.set_value(); }
			));

			boost::regex include_re(R"(\s*#\s*include\s.*)");
			IWriteBufferPtr preprocessor_stdin = preprocessor->GetStdin();

			std::ifstream src_file(_filename);
			std::string line;
			while (std::getline(src_file, line))
			{
				std::string str;
				boost::smatch m;
				if (!boost::regex_match(line, m, include_re))
					str = line;
				str += "\n";
				preprocessor_stdin->Write(ByteArray(str.begin(), str.end()));
			}
			preprocessor_stdin->Close();

			stdout_closed.get_future().wait();
			stderr_closed.get_future().wait();
			preprocessor.reset();

			HIDE_CHECK(*ret_code == 0, std::runtime_error(StringBuilder() % "Preprocessor failed, ret code: " % *ret_code));
		}

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

					entries.push_back(std::make_shared<CppCTagsIndexEntry>(name, scope, kind, Location(_filename, line, 1)));
				}
			);

		return std::make_shared<CppCTagsPartialIndex>(entries);
	}


	IPartialIndexPtr CppCTagsIndexer::LoadIndex(const std::string& filename)
	{
		using namespace boost::property_tree;

		ptree root;
		std::ifstream f(filename);
		read_json(f, root);

		PTreeReader wr(root);
		std::vector<CppCTagsIndexEntry::SerializationProxy> ctags_entries;
		PTreeReader(root).Read("entries", ctags_entries);

		std::vector<IIndexEntryPtr> entries;
		for (const auto& e : ctags_entries)
			entries.push_back(std::make_shared<CppCTagsIndexEntry>(e.ToEntry(_filename)));

		return std::make_shared<CppCTagsPartialIndex>(entries);
	}

}
