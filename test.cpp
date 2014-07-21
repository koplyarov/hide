#include <iostream>
#include <fstream>
#include <vector>

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include <hide/lang_plugins/cpp/clang/CompilationDatabase.h>
#include <hide/lang_plugins/cpp/clang/Index.h>


using namespace hide::cpp::clang;

class DumpVisitor : public VisitorBase<DumpVisitor>
{
	std::string _indent;

public:
	DumpVisitor(const std::string& indent = "")
		: _indent(indent)
	{ }

	CXChildVisitResult Visit(Cursor c, Cursor p) const
	{
		if (c.GetLocation().IsFromMainFile() && c.IsDefinition())
		{
			std::cout << _indent << "kind: " << CXCursorKindToString(c.GetKind());

			std::string type = c.GetType().GetSpelling();
			if (!type.empty())
				std::cout << ", type: " << type;

			std::string spelling = c.GetSpelling();
			if (!spelling.empty())
				std::cout << ", spelling: " << spelling;

			std::string displayName = c.GetDisplayName();
			if (!displayName.empty())
				std::cout << ", displayName: " << displayName;

			std::cout << std::endl;
		}

		c.VisitChildren(DumpVisitor(_indent + "  "));

		return CXChildVisit_Continue;
	}
};


class FileReader
{
private:
	std::string		_filename;

public:
	FileReader(const std::string& filename) : _filename(filename) { }

	std::string ReadLine(size_t lineNum) const
	{
		std::fstream file(_filename);
		size_t line_num = 0;
		std::string res;
		do
		{ HIDE_CHECK(std::getline(file, res, '\n'), std::runtime_error("Could not read line from a file!")); }
		while (line_num++ < lineNum);
		return res;
	}
};
HIDE_DECLARE_PTR(FileReader);

namespace fs = boost::filesystem;

fs::path relativePath( const fs::path &path, const fs::path &relative_to )
{
    // create absolute paths
    fs::path p = fs::absolute(path);
    fs::path r = fs::absolute(relative_to);

    // if root paths are different, return absolute path
    if( p.root_path() != r.root_path() )
        return p;

    // initialize relative path
    fs::path result;

    // find out where the two paths diverge
    fs::path::const_iterator itr_path = p.begin();
    fs::path::const_iterator itr_relative_to = r.begin();
    while( *itr_path == *itr_relative_to && itr_path != p.end() && itr_relative_to != r.end() ) {
        ++itr_path;
        ++itr_relative_to;
    }

    // add "../" for each remaining token in relative_to
    if( itr_relative_to != r.end() ) {
        ++itr_relative_to;
        while( itr_relative_to != r.end() ) {
            result /= "..";
            ++itr_relative_to;
        }
    }

    // add remaining path
    while( itr_path != p.end() ) {
        result /= *itr_path;
        ++itr_path;
    }

    return result;
}


class TagEntry
{
private:
	FileReaderPtr				_fileReader;
	std::string					_name;
	boost::filesystem::path		_filename;
	std::string					_exCmd;

public:
	TagEntry(Cursor c)
		:	_fileReader(new FileReader(c.GetLocation().GetFile().GetFileName())),
			_name(c.GetSpelling()),
			_filename(c.GetLocation().GetFile().GetFileName()),
			_exCmd(MakeExCommand(c.GetLocation()))
	{ }

	void WriteToFile(std::ostream& s) const
	{ s << _name << '\t' << relativePath(_filename, boost::filesystem::current_path()) << '\t' << _exCmd << std::endl; }

	bool operator < (const TagEntry& other) const
	{ return _name < other._name; }

private:
	std::string MakeExCommand(const SourceLocation& loc)
	{
		static boost::regex escape_re("([/])", boost::regex_constants::extended);
		std::string line =  _fileReader->ReadLine(loc.GetLineNum() - 1);
		return "/^" + boost::regex_replace(line, escape_re, "\\\\$1") + "$/";
	}
};


typedef std::vector<TagEntry>	Tags;


class TagsVisitor : public VisitorBase<TagsVisitor>
{
	Tags*			_tags;
	std::string		_parent;

public:
	TagsVisitor(Tags& tags, const std::string& parent = "")
		: _tags(&tags), _parent(parent)
	{ }

	CXChildVisitResult Visit(Cursor c, Cursor p) const
	{
		std::vector<CXCursorKind> definitely_nots = { CXCursor_CXXAccessSpecifier, CXCursor_TemplateTypeParameter, CXCursor_UnexposedDecl };
		std::vector<CXCursorKind> funcs = { CXCursor_FunctionDecl, CXCursor_CXXMethod, CXCursor_FunctionTemplate, CXCursor_Constructor };

		std::string new_parent = _parent;

		bool definitely_not = std::find(definitely_nots.begin(), definitely_nots.end(), c.GetKind()) == definitely_nots.end();
		bool is_func = std::find(funcs.begin(), funcs.end(), c.GetKind()) != funcs.end();

		if (c.GetLocation().IsFromMainFile() && ((c.IsDefinition() && definitely_not) || is_func))
		{
			_tags->push_back(TagEntry(c));
			new_parent += c.GetSpelling() + "::";
		}

		if (!is_func && c.GetLocation().IsFromMainFile())
			c.VisitChildren(TagsVisitor(*_tags, new_parent));

		return CXChildVisit_Continue;
	}
};


int main()
{
	try
	{
		std::cout << "Hi!" << std::endl;

		std::vector<std::string> cmd_line_args;

		std::string filename = (boost::filesystem::current_path() / "test.cpp").native();
		std::cout << "filename: " << filename << std::endl;

		CompilationDatabasePtr db = CompilationDatabase::FromDirectory("./");
		CompileCommandsPtr cmds = db->GetCompileCommands(filename);
		for (size_t i = 0; i < cmds->GetSize(); ++i)
		{
			CompileCommandPtr cmd = cmds->GetCompileCommand(i);
			std::cout << "dir: " << cmd->GetDirectory() << std::endl;
			std::cout << "args: " << std::endl;
			for (size_t j = 0; j < cmd->GetNumArgs(); ++j)
			{
				std::cout << "    " << cmd->GetArg(j) << std::endl;
				if (j > 0 && j < cmd->GetNumArgs() - 1)
					cmd_line_args.push_back(cmd->GetArg(j));
			}
		}

		std::vector<std::string> builtin_includes = {
			"/usr/include",
			"/usr/lib/llvm-3.5/include",
			"/usr/include/c++/4.8",
			"/usr/include/x86_64-linux-gnu/c++/4.8",
			"/usr/include/c++/4.8/backward",
			//"/usr/lib/gcc/x86_64-linux-gnu/4.8/include",
			"/usr/local/include",
			"/usr/lib/gcc/x86_64-linux-gnu/4.8/include-fixed",
			"/usr/include/x86_64-linux-gnu",
			"/usr/lib/llvm-3.5/lib/clang/3.5/include"
		};

		std::transform(builtin_includes.begin(), builtin_includes.end(), std::back_inserter(cmd_line_args), [] (const std::string& p) { return "-I" + p; });

		IndexPtr index = Index::Create(true, true);
		TranslationUnitPtr tu = index->ParseTranslationUnit(filename, cmd_line_args, std::vector<UnsavedFile>(), 0);

		Tags tags;
		tu->GetCursor().VisitChildren(TagsVisitor(tags));

		std::sort(tags.begin(), tags.end());

		for (TagEntry t: tags)
			t.WriteToFile(std::cout);

		//tu->GetCursor().VisitChildren(DumpVisitor());

		std::cout << "Memory used for this translation unit: " << tu->GetResourceUsage().GetTotal() << std::endl;
		tu->Reparse(std::vector<UnsavedFile>(), 0);
		std::cout << "Memory used for this translation unit: " << tu->GetResourceUsage().GetTotal() << std::endl;
	}
	catch (const std::exception& ex)
	{
		std::cerr << "ERROR: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
