#include <iostream>
#include <vector>

#include <ClangWrappers.h>


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
		std::vector<CXCursorKind> definitely_not = { CXCursor_CXXAccessSpecifier, CXCursor_TemplateTypeParameter, CXCursor_UnexposedDecl };
		std::vector<CXCursorKind> definitely_yes = { CXCursor_FunctionDecl, CXCursor_CXXMethod, CXCursor_FunctionTemplate };

		std::string new_parent = _parent;

		if (c.GetLocation().IsFromMainFile() && ((c.IsDefinition() && std::find(definitely_not.begin(), definitely_not.end(), c.GetKind()) == definitely_not.end()) || std::find(definitely_yes.begin(), definitely_yes.end(), c.GetKind()) != definitely_yes.end()))
		{
			std::cout << _parent << c.GetSpelling() << "\t" << c.GetLocation().GetFile().GetFileName() << "" << std::endl;
			new_parent += c.GetSpelling() + "::";
		}

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

		CompilationDatabasePtr db = CompilationDatabase::FromDirectory("./");
		CompileCommandsPtr cmds = db->GetCompileCommands("/home/koplyarov/work/clang_test/test.cpp");
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
		TranslationUnitPtr tu = index->ParseTranslationUnit("/home/koplyarov/work/clang_test/test.cpp", cmd_line_args, std::vector<UnsavedFile>(), 0);
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
