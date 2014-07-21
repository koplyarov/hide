#ifndef CLANG_COMPILATIONDATABASE_H
#define CLANG_COMPILATIONDATABASE_H


#include <hide/Utils.h>
#include <hide/lang_plugins/cpp/clang/Common.h>
#include <hide/lang_plugins/cpp/clang/String.h>


namespace hide {
namespace cpp {
namespace clang
{

	BEGIN_CLANG_WRAPPER_NO_DISPOSE(CompileCommand)
		std::string GetDirectory() const { return String(clang_CompileCommand_getDirectory(_raw)); }

		size_t GetNumArgs() const { return clang_CompileCommand_getNumArgs(_raw); }
		std::string GetArg(size_t index) const { return String(clang_CompileCommand_getArg(_raw, index)); }
	END_CLANG_WRAPPER();


	BEGIN_CLANG_WRAPPER(CompileCommands, clang_CompileCommands_dispose)
		size_t GetSize() const { return clang_CompileCommands_getSize(_raw); }

		CompileCommandPtr GetCompileCommand(size_t index) const
		{ return std::make_shared<CompileCommand>(REQUIRE_NOT_NULL(clang_CompileCommands_getCommand(_raw, index))); }
	END_CLANG_WRAPPER();


	BEGIN_CLANG_WRAPPER(CompilationDatabase, clang_CompilationDatabase_dispose)
		static CompilationDatabasePtr FromDirectory(const std::string& dir)
		{
			CXCompilationDatabase_Error err = CXCompilationDatabase_NoError;
			CXCompilationDatabase raw_db = clang_CompilationDatabase_fromDirectory(dir.c_str(), &err);
			HIDE_CHECK(err == CXCompilationDatabase_NoError, std::runtime_error("clang_CompilationDatabase_fromDirectory failed!"));
			return std::make_shared<CompilationDatabase>(raw_db);
		}

		CompileCommandsPtr GetCompileCommands(const std::string& completeFilename) const
		{ return std::make_shared<CompileCommands>(REQUIRE_NOT_NULL(clang_CompilationDatabase_getCompileCommands(_raw, completeFilename.c_str()))); }
	END_CLANG_WRAPPER();

}}}

#endif
