#include <unistd.h>

#include <clang/CompilationDatabase.h>
#include <clang/Index.h>

#include <iostream>
#include <vector>


using namespace clang;

class TestVisitor : public VisitorBase<TestVisitor>
{
	std::string _indent;

public:
	TestVisitor(const std::string& indent = "")
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

		c.VisitChildren(TestVisitor(_indent + "  "));

		return CXChildVisit_Continue;
	}
};

class TagsVisitor : public VisitorBase<TagsVisitor>
{
	std::string _parent;

public:
	TagsVisitor(const std::string& parent = "")
		: _parent(parent)
	{ }

	CXChildVisitResult Visit(Cursor c, Cursor p) const
	{
		std::vector<CXCursorKind> definitely_nots = { CXCursor_CXXAccessSpecifier, CXCursor_TemplateTypeParameter, CXCursor_UnexposedDecl };
		std::vector<CXCursorKind> funcs = { CXCursor_FunctionDecl, CXCursor_CXXMethod, CXCursor_FunctionTemplate };

		std::string new_parent = _parent;

		bool definitely_not = std::find(definitely_nots.begin(), definitely_nots.end(), c.GetKind()) == definitely_nots.end();
		bool is_func = std::find(funcs.begin(), funcs.end(), c.GetKind()) != funcs.end();

		if (c.GetLocation().IsFromMainFile() && ((c.IsDefinition() && definitely_not) || is_func))
		{
			std::cout << _parent << c.GetSpelling() << "\t" << c.GetLocation().GetFile().GetFileName() << "" << std::endl;
			new_parent += c.GetSpelling() + "::";
		}

		if (!is_func && c.GetLocation().IsFromMainFile())
			c.VisitChildren(TagsVisitor(new_parent));

		return CXChildVisit_Continue;
	}
};


int main()
{
	try
	{
		std::cout << "Hi!" << std::endl;

		std::vector<std::string> cmd_line_args;

		std::string filename = std::string(get_current_dir_name()) + "/test.cpp";
		std::cout << "filename: " << filename << std::endl;

		CompilationDatabasePtr db = CompilationDatabase::FromDirectory("./");
		CompileCommandsPtr cmds = db->GetCompileCommands(filename);
		for (size_t i = 0; i < cmds->GetSize(); ++i)
		{
			CompileCommandPtr cmd = cmds->GetCompileCommand(i);
			std::cout << "cmd: " << cmd->GetDirectory() << std::endl;
			std::cout << "args: " << std::endl;
			for (size_t j = 0; j < cmd->GetNumArgs(); ++j)
			{
				std::cout << "    " << cmd->GetArg(j) << std::endl;
				if (j > 0 && j < cmd->GetNumArgs() - 1)
					cmd_line_args.push_back(cmd->GetArg(j));
			}
		}

		IndexPtr index = Index::Create(true, true);
		TranslationUnitPtr tu = index->ParseTranslationUnit(filename, cmd_line_args, std::vector<UnsavedFile>(), 0);
		tu->GetCursor().VisitChildren(TagsVisitor());
		//tu->GetCursor().VisitChildren(TestVisitor());
	}
	catch (const std::exception& ex)
	{
		std::cerr << "ERROR: " << ex.what() << std::endl;
		return 1;
	}

	return 0;
}
