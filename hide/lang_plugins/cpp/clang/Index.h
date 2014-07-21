#ifndef CLANG_INDEX_H
#define CLANG_INDEX_H


#include <algorithm>
#include <functional>
#include <vector>

#include <hide/lang_plugins/cpp/clang/Common.h>
#include <hide/lang_plugins/cpp/clang/String.h>


namespace hide {
namespace cpp {
namespace clang
{

	std::string CXCursorKindToString(CXCursorKind kind);


	class UnsavedFile
	{
		CXUnsavedFile	_raw;

	public:
		UnsavedFile() { BOOST_THROW_EXCEPTION(std::runtime_error("Not implemented")); }

		CXUnsavedFile GetRaw() const { return _raw; }
	};


	template < typename VisitorType_ >
	class VisitorBase
	{
	public:
		static CXChildVisitResult VisitorFunc(CXCursor cursor, CXCursor parent, CXClientData clientData);
	};

	BEGIN_CLANG_WRAPPER_NO_DISPOSE(Type)
		std::string GetSpelling() const { return String(clang_getTypeSpelling(_raw)); }
	END_CLANG_WRAPPER();


	BEGIN_CLANG_WRAPPER_NO_DISPOSE(Module);
		std::string GetName() const { return String(clang_Module_getName(_raw)); }
		std::string GetFullName() const { return String(clang_Module_getFullName(_raw)); }
	END_CLANG_WRAPPER();


	BEGIN_CLANG_WRAPPER_NO_DISPOSE(File)
		std::string GetFileName() const { return String(clang_getFileName(_raw)); }
		time_t GetFileTime() const { return clang_getFileTime(_raw); }
		CXFileUniqueID GetUniqueID() const { CXFileUniqueID res = { }; HIDE_CHECK(clang_getFileUniqueID(_raw, &res) == 0, std::runtime_error("clang_getFileUniqueID failed!")); return res; }
	END_CLANG_WRAPPER();


	BEGIN_CLANG_WRAPPER_NO_DISPOSE(SourceLocation)
		static SourceLocation GetNull() { return SourceLocation(clang_getNullLocation()); }

		void GetParams(File& outFile, unsigned& outLine, unsigned& outColumn, unsigned& outOffset)
		{ CXFile f; clang_getExpansionLocation(_raw, &f, &outLine, &outColumn, &outOffset); outFile = f; }

		File GetFile() const { CXFile res; clang_getExpansionLocation(_raw, &res, NULL, NULL, NULL); return res; }
		size_t GetLineNum() const { unsigned res; clang_getExpansionLocation(_raw, NULL, &res, NULL, NULL); return res; }
		size_t GetColumnNum() const { unsigned res; clang_getExpansionLocation(_raw, NULL, NULL, &res, NULL); return res; }
		size_t GetOffset() const { unsigned res; clang_getExpansionLocation(_raw, NULL, NULL, NULL, &res); return res; }

		bool IsInSystemHeader() const { return clang_Location_isInSystemHeader(_raw); }
		bool IsFromMainFile() const { return clang_Location_isFromMainFile(_raw); }

		bool operator == (const SourceLocation& other) const { return clang_equalLocations(_raw, other._raw); }
		bool operator != (const SourceLocation& other) const { return !(*this == other); }
	END_CLANG_WRAPPER();


	BEGIN_CLANG_WRAPPER_NO_DISPOSE(Cursor)
		CXCursorKind GetKind() const { return clang_getCursorKind(_raw); }

		template < typename VisitorType_ >
		unsigned VisitChildren(const VisitorType_& visitor) const
		{ return clang_visitChildren(_raw, VisitorType_::VisitorFunc, (CXClientData)&visitor); }

		bool IsDeclaration() const { return clang_isDeclaration(GetKind()); }
		bool IsDefinition() const { return clang_isCursorDefinition(_raw); }
		Type GetType() const { return clang_getCursorType(_raw); }

		Cursor GetLexicalParent() const { return clang_getCursorLexicalParent(_raw); }
		Cursor GetSemanticParent() const { return clang_getCursorSemanticParent(_raw); }

		Module GetModule() const { return clang_Cursor_getModule(_raw); }
		SourceLocation GetLocation() const { return clang_getCursorLocation(_raw); }

		std::string GetSpelling() const { return String(clang_getCursorSpelling(_raw)); }
		std::string GetDisplayName() const { return String(clang_getCursorDisplayName(_raw)); }

		static Cursor GetNull() { return Cursor(clang_getNullCursor()); }

		bool operator == (const Cursor& other) const { return clang_equalCursors(_raw, other._raw); }
		bool operator != (const Cursor& other) const { return !(*this == other); }
	END_CLANG_WRAPPER();

	template < typename VisitorType_ >
	CXChildVisitResult VisitorBase<VisitorType_>::VisitorFunc(CXCursor cursor, CXCursor parent, CXClientData clientData)
	{
		const VisitorType_* self = (const VisitorType_*)clientData;
		return self->Visit(Cursor(cursor), Cursor(parent));
	}


	BEGIN_CLANG_WRAPPER(TUResourceUsage, clang_disposeCXTUResourceUsage)
		uint64_t GetTotal() const
		{
			uint64_t result = 0;
			for (size_t i = 0; i < _raw.numEntries; ++i)
				result += _raw.entries[i].amount;
			return result;
		}
	END_CLANG_WRAPPER();


	BEGIN_CLANG_WRAPPER(TranslationUnit, clang_disposeTranslationUnit)
		Cursor GetCursor() const { return clang_getTranslationUnitCursor(_raw); }
		Cursor GetCursor(const SourceLocation& loc) const { return clang_getCursor(_raw, loc.GetRaw()); }
		SourceLocation GetLocation(const File& file, size_t line, size_t column) const { return clang_getLocation(_raw, file.GetRaw(), line, column); }
		File GetFile(const std::string& filename) const { return clang_getFile(_raw, filename.c_str()); }
		TUResourceUsage GetResourceUsage() const { return clang_getCXTUResourceUsage(_raw); }

		void Reparse(std::vector<UnsavedFile> unsavedFiles, unsigned options)
		{
			std::vector<CXUnsavedFile> unsaved_files;
			unsaved_files.reserve(unsavedFiles.size());
			std::transform(unsavedFiles.begin(), unsavedFiles.end(), std::back_inserter(unsaved_files), std::function<CXUnsavedFile(const UnsavedFile&)>(&UnsavedFile::GetRaw));

			HIDE_CHECK(clang_reparseTranslationUnit(_raw, unsaved_files.size(), unsaved_files.data(), options) == 0, std::runtime_error("clang_reparseTranslationUnit failed!"));
		}
	END_CLANG_WRAPPER();


	BEGIN_CLANG_WRAPPER(Index, clang_disposeIndex)
		static IndexPtr Create(bool excludeDeclarationsFromPCH, bool displayDiagnostics)
		{ return std::make_shared<Index>(REQUIRE_NOT_NULL(clang_createIndex(excludeDeclarationsFromPCH, displayDiagnostics))); }

		TranslationUnitPtr ParseTranslationUnit(const std::string& sourceFilename, std::vector<std::string> commandLineArgs, std::vector<UnsavedFile> unsavedFiles, unsigned options)
		{
			std::vector<const char*> cmd_line_args;
			cmd_line_args.reserve(commandLineArgs.size());
			std::transform(commandLineArgs.begin(), commandLineArgs.end(), std::back_inserter(cmd_line_args), std::function<const char*(const std::string&)>(&std::string::c_str));

			std::vector<CXUnsavedFile> unsaved_files;
			unsaved_files.reserve(unsavedFiles.size());
			std::transform(unsavedFiles.begin(), unsavedFiles.end(), std::back_inserter(unsaved_files), std::function<CXUnsavedFile(const UnsavedFile&)>(&UnsavedFile::GetRaw));

			return std::make_shared<TranslationUnit>(REQUIRE_NOT_NULL(clang_parseTranslationUnit(_raw, sourceFilename.c_str(), cmd_line_args.data(), cmd_line_args.size(), unsaved_files.data(), unsaved_files.size(), options)));
		}
	END_CLANG_WRAPPER();

}}}

#endif
