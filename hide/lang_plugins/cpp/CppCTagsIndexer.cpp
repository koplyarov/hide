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
#include <hide/utils/PipeLinesReader.h>


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


	std::set<std::string> CppCTagsIndexer::GetFileDefines(const std::string& filename)
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


	StringArray CppCTagsIndexer::GetStdIncludePaths()
	{
		StringArray result;
		bool include_paths_section = false;

		StringArray params = { "-v", "-x", "c++", "-E", "-" };
		ExecutablePtr gcc = std::make_shared<Executable>("g++", params,
			std::make_shared<PipeLinesReader>([](const std::string& s) { }),
			std::make_shared<PipeLinesReader>(
				[&](const std::string& s)
				{
					if (s == "End of search list.")
						include_paths_section = false;
					if (include_paths_section)
						result.push_back(boost::algorithm::trim_copy(s));
					if (s == "#include <...> search starts here:")
						include_paths_section = true;
				}
			)
		);

		boost::optional<int> ret_code;
		gcc->AddListener(std::make_shared<FuncExecutableListener>([&](int retCode) { ret_code = retCode; }));

		gcc->GetStdin()->Close();
		gcc.reset();

		return result;
	}


	void CppCTagsIndexer::GenerateIncludeFile(const boost::filesystem::path& dst)
	{
		StringArray std_include_paths = GetStdIncludePaths();
		if (std_include_paths.empty())
			s_logger.Warning() << "Empty default include paths!";

		std::set<std::string> file_defines = GetFileDefines(_filename);

		StringArray preprocessor_params = { "-xc++", "-dM", "-o-", _filename };
		boost::transform(std_include_paths, std::back_inserter(preprocessor_params), [](const std::string& s) { return "-I" + s; });

		// TODO: get these parameters from somewhere else
		preprocessor_params.insert(preprocessor_params.end(), { "-DHIDE_PLATFORM_POSIX=1", "-Dhide_EXPORTS", "-I/usr/include/python2.7", "-I/usr/include/x86_64-linux-gnu/python2.7", "-I/usr/lib/llvm-3.5/include", "-I.", "-std=c++11" });

		boost::regex define_re(R"(\s*#\s*define\s+([A-Za-z_][A-Za-z0-9_]*).*)");
		std::ofstream file_to_include_f(dst.string(), std::ios::binary | std::ios::out);

		ExecutablePtr preprocessor = std::make_shared<Executable>("cpp", preprocessor_params,
			std::make_shared<PipeLinesReader>(
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
				}),
			s_logger
		);

		boost::optional<int> ret_code;
		preprocessor->AddListener(std::make_shared<FuncExecutableListener>([&](int retCode) { ret_code = retCode; }));

		preprocessor->GetStdin()->Close();
		preprocessor.reset();

		HIDE_CHECK(*ret_code == 0, std::runtime_error(StringBuilder() % "Preprocessor failed, ret code: " % *ret_code));
	}


	void CppCTagsIndexer::PreprocessFile(const boost::filesystem::path& includeFile, const boost::filesystem::path& dst)
	{
		StringArray preprocessor_params = { "-w", "-xc++", "-std=c++11", "-include", includeFile.string(), "-o-", "-" };

		boost::regex pp_stuff_re(R"X(\s*#\s+(\d+)\s+"([^"]+)".*)X"); // " This comment fixes vim syntax highlight =)
		std::ofstream preprocessor_result_f(dst.string(), std::ios::binary | std::ios::out);
		int line_num = 1;
		ExecutablePtr preprocessor = std::make_shared<Executable>("cpp", preprocessor_params,
			std::make_shared<PipeLinesReader>(
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
				}),
			s_logger
		);

		boost::optional<int> ret_code;
		preprocessor->AddListener(std::make_shared<FuncExecutableListener>([&](int retCode) { ret_code = retCode; }));

		boost::regex include_re(R"(\s*#\s*include\s.*)");
		IPipeWriteEndPtr preprocessor_stdin = preprocessor->GetStdin();
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

		preprocessor.reset();

		HIDE_CHECK(*ret_code == 0, std::runtime_error(StringBuilder() % "Preprocessor failed, ret code: " % *ret_code));
	}


	IPartialIndexPtr CppCTagsIndexer::BuildIndex()
	{
		using namespace boost::filesystem;

		IIndexEntryPtrArray entries;

		StringArray scope_fields = { "class", "struct", "namespace", "enum" };
		std::map<std::string, IndexEntryKind> kinds_map =
			{
				{ "namespace",	IndexEntryKind::Namespace },
				{ "typedef",	IndexEntryKind::Type },
				{ "class",		IndexEntryKind::Type },
				{ "struct",		IndexEntryKind::Type },
				{ "enum",		IndexEntryKind::Type },
				{ "member",		IndexEntryKind::Variable },
				{ "function",	IndexEntryKind::Function },
				{ "prototype",	IndexEntryKind::Function },
				{ "macro",		IndexEntryKind::Macro }
			};

		auto tag_entry_builder =
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
				};

		try
		{
			path file_to_include = path(s_tempDirectory) / path(_filename + ".cppIndexer.inc");
			BOOST_SCOPE_EXIT_ALL(&) {
				RemoveFileAndParentDirectories(file_to_include, s_tempDirectory);
			};

			path preprocessor_result = path(s_tempDirectory) / path(_filename + ".cppIndexer.cpp");
			boost::filesystem::create_directories(preprocessor_result.parent_path());
			BOOST_SCOPE_EXIT_ALL(&) {
				RemoveFileAndParentDirectories(preprocessor_result, s_tempDirectory);
			};

			GenerateIncludeFile(file_to_include);
			PreprocessFile(file_to_include, preprocessor_result);

			CTagsInvoker({ "-f-", "--excmd=number", "--sort=no", "--fields=+aimSztK", "--c-kinds=+p", preprocessor_result.string() }, tag_entry_builder);
			CTagsInvoker({ "-f-", "--excmd=number", "--sort=no", "--fields=+aimSztK", "--c-kinds=d", _filename }, tag_entry_builder);
		}
		catch (const std::exception& ex)
		{
			s_logger.Warning() << "Could get tags from the preprocessed file: " << ex;
			entries.clear();
			CTagsInvoker({ "-f-", "--excmd=number", "--sort=no", "--fields=+aimSztK", _filename }, tag_entry_builder);
		}

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
