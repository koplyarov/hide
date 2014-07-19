#ifndef CLANGWRAPPERS_H
#define CLANGWRAPPERS_H


#include <algorithm>
#include <functional>
#include <vector>

#include <clang-c/CXCompilationDatabase.h>
#include <clang-c/Index.h>

#include <Utils.h>


namespace clang
{

#define BEGIN_CLANG_WRAPPER_NO_DISPOSE(Name_) \
	class Name_; \
	DECLARE_PTR(Name_); \
	class Name_ \
	{ \
		CX##Name_	_raw; \
	public: \
		Name_(CX##Name_ raw) : _raw(raw) { } \
		CX##Name_ GetRaw() const { return _raw; }

#define BEGIN_CLANG_WRAPPER(Name_, DisposeFunc_) \
	BEGIN_CLANG_WRAPPER_NO_DISPOSE(Name_) \
		~Name_() { DisposeFunc_(_raw); }

#define END_CLANG_WRAPPER() \
	}


	class StringWrapper
	{
		CXString	_raw;

	public:
		StringWrapper(CXString raw) : _raw(raw) { REQUIRE_NOT_NULL(_raw.data); }
		~StringWrapper() { clang_disposeString(_raw); }
		operator std::string() const { return clang_getCString(_raw); }
	};

	BEGIN_CLANG_WRAPPER_NO_DISPOSE(CompileCommand)
		std::string GetDirectory() const { return StringWrapper(clang_CompileCommand_getDirectory(_raw)); }

		size_t GetNumArgs() const { return clang_CompileCommand_getNumArgs(_raw); }
		std::string GetArg(size_t index) const { return StringWrapper(clang_CompileCommand_getArg(_raw, index)); }
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
			CHECK(err == CXCompilationDatabase_NoError, std::runtime_error("clang_CompilationDatabase_fromDirectory failed!"));
			return std::make_shared<CompilationDatabase>(raw_db);
		}

		CompileCommandsPtr GetCompileCommands(const std::string& completeFilename) const
		{ return std::make_shared<CompileCommands>(REQUIRE_NOT_NULL(clang_CompilationDatabase_getCompileCommands(_raw, completeFilename.c_str()))); }
	END_CLANG_WRAPPER();


	std::string CXCursorKindToString(CXCursorKind kind)
	{
#define CX_CURSOR_TOSTRING(Val_) case CXCursor_##Val_: return #Val_
		switch (kind)
		{
		CX_CURSOR_TOSTRING(UnexposedDecl);
		CX_CURSOR_TOSTRING(StructDecl);
		CX_CURSOR_TOSTRING(UnionDecl);
		CX_CURSOR_TOSTRING(ClassDecl);
		CX_CURSOR_TOSTRING(EnumDecl);
		CX_CURSOR_TOSTRING(FieldDecl);
		CX_CURSOR_TOSTRING(EnumConstantDecl);
		CX_CURSOR_TOSTRING(FunctionDecl);
		CX_CURSOR_TOSTRING(VarDecl);
		CX_CURSOR_TOSTRING(ParmDecl);
		CX_CURSOR_TOSTRING(ObjCInterfaceDecl);
		CX_CURSOR_TOSTRING(ObjCCategoryDecl);
		CX_CURSOR_TOSTRING(ObjCProtocolDecl);
		CX_CURSOR_TOSTRING(ObjCPropertyDecl);
		CX_CURSOR_TOSTRING(ObjCIvarDecl);
		CX_CURSOR_TOSTRING(ObjCInstanceMethodDecl);
		CX_CURSOR_TOSTRING(ObjCClassMethodDecl);
		CX_CURSOR_TOSTRING(ObjCImplementationDecl);
		CX_CURSOR_TOSTRING(ObjCCategoryImplDecl);
		CX_CURSOR_TOSTRING(TypedefDecl);
		CX_CURSOR_TOSTRING(CXXMethod);
		CX_CURSOR_TOSTRING(Namespace);
		CX_CURSOR_TOSTRING(LinkageSpec);
		CX_CURSOR_TOSTRING(Constructor);
		CX_CURSOR_TOSTRING(Destructor);
		CX_CURSOR_TOSTRING(ConversionFunction);
		CX_CURSOR_TOSTRING(TemplateTypeParameter);
		CX_CURSOR_TOSTRING(NonTypeTemplateParameter);
		CX_CURSOR_TOSTRING(TemplateTemplateParameter);
		CX_CURSOR_TOSTRING(FunctionTemplate);
		CX_CURSOR_TOSTRING(ClassTemplate);
		CX_CURSOR_TOSTRING(ClassTemplatePartialSpecialization);
		CX_CURSOR_TOSTRING(NamespaceAlias);
		CX_CURSOR_TOSTRING(UsingDirective);
		CX_CURSOR_TOSTRING(UsingDeclaration);
		CX_CURSOR_TOSTRING(TypeAliasDecl);
		CX_CURSOR_TOSTRING(ObjCSynthesizeDecl);
		CX_CURSOR_TOSTRING(ObjCDynamicDecl);
		CX_CURSOR_TOSTRING(CXXAccessSpecifier);
		CX_CURSOR_TOSTRING(FirstRef);
		CX_CURSOR_TOSTRING(ObjCProtocolRef);
		CX_CURSOR_TOSTRING(ObjCClassRef);
		CX_CURSOR_TOSTRING(TypeRef);
		CX_CURSOR_TOSTRING(CXXBaseSpecifier);
		CX_CURSOR_TOSTRING(TemplateRef);
		CX_CURSOR_TOSTRING(NamespaceRef);
		CX_CURSOR_TOSTRING(MemberRef);
		CX_CURSOR_TOSTRING(LabelRef);
		CX_CURSOR_TOSTRING(OverloadedDeclRef);
		CX_CURSOR_TOSTRING(VariableRef);
		CX_CURSOR_TOSTRING(FirstInvalid);
		CX_CURSOR_TOSTRING(NoDeclFound);
		CX_CURSOR_TOSTRING(NotImplemented);
		CX_CURSOR_TOSTRING(InvalidCode);
		CX_CURSOR_TOSTRING(FirstExpr);
		CX_CURSOR_TOSTRING(DeclRefExpr);
		CX_CURSOR_TOSTRING(MemberRefExpr);
		CX_CURSOR_TOSTRING(CallExpr);
		CX_CURSOR_TOSTRING(ObjCMessageExpr);
		CX_CURSOR_TOSTRING(BlockExpr);
		CX_CURSOR_TOSTRING(IntegerLiteral);
		CX_CURSOR_TOSTRING(FloatingLiteral);
		CX_CURSOR_TOSTRING(ImaginaryLiteral);
		CX_CURSOR_TOSTRING(StringLiteral);
		CX_CURSOR_TOSTRING(CharacterLiteral);
		CX_CURSOR_TOSTRING(ParenExpr);
		CX_CURSOR_TOSTRING(UnaryOperator);
		CX_CURSOR_TOSTRING(ArraySubscriptExpr);
		CX_CURSOR_TOSTRING(BinaryOperator);
		CX_CURSOR_TOSTRING(CompoundAssignOperator);
		CX_CURSOR_TOSTRING(ConditionalOperator);
		CX_CURSOR_TOSTRING(CStyleCastExpr);
		CX_CURSOR_TOSTRING(CompoundLiteralExpr);
		CX_CURSOR_TOSTRING(InitListExpr);
		CX_CURSOR_TOSTRING(AddrLabelExpr);
		CX_CURSOR_TOSTRING(StmtExpr);
		CX_CURSOR_TOSTRING(GenericSelectionExpr);
		CX_CURSOR_TOSTRING(GNUNullExpr);
		CX_CURSOR_TOSTRING(CXXStaticCastExpr);
		CX_CURSOR_TOSTRING(CXXDynamicCastExpr);
		CX_CURSOR_TOSTRING(CXXReinterpretCastExpr);
		CX_CURSOR_TOSTRING(CXXConstCastExpr);
		CX_CURSOR_TOSTRING(CXXFunctionalCastExpr);
		CX_CURSOR_TOSTRING(CXXTypeidExpr);
		CX_CURSOR_TOSTRING(CXXBoolLiteralExpr);
		CX_CURSOR_TOSTRING(CXXNullPtrLiteralExpr);
		CX_CURSOR_TOSTRING(CXXThisExpr);
		CX_CURSOR_TOSTRING(CXXThrowExpr);
		CX_CURSOR_TOSTRING(CXXNewExpr);
		CX_CURSOR_TOSTRING(CXXDeleteExpr);
		CX_CURSOR_TOSTRING(UnaryExpr);
		CX_CURSOR_TOSTRING(ObjCStringLiteral);
		CX_CURSOR_TOSTRING(ObjCEncodeExpr);
		CX_CURSOR_TOSTRING(ObjCSelectorExpr);
		CX_CURSOR_TOSTRING(ObjCProtocolExpr);
		CX_CURSOR_TOSTRING(ObjCBridgedCastExpr);
		CX_CURSOR_TOSTRING(PackExpansionExpr);
		CX_CURSOR_TOSTRING(SizeOfPackExpr);
		CX_CURSOR_TOSTRING(LambdaExpr);
		CX_CURSOR_TOSTRING(ObjCBoolLiteralExpr);
		CX_CURSOR_TOSTRING(ObjCSelfExpr);
		CX_CURSOR_TOSTRING(FirstStmt);
		CX_CURSOR_TOSTRING(LabelStmt);
		CX_CURSOR_TOSTRING(CompoundStmt);
		CX_CURSOR_TOSTRING(CaseStmt);
		CX_CURSOR_TOSTRING(DefaultStmt);
		CX_CURSOR_TOSTRING(IfStmt);
		CX_CURSOR_TOSTRING(SwitchStmt);
		CX_CURSOR_TOSTRING(WhileStmt);
		CX_CURSOR_TOSTRING(DoStmt);
		CX_CURSOR_TOSTRING(ForStmt);
		CX_CURSOR_TOSTRING(GotoStmt);
		CX_CURSOR_TOSTRING(IndirectGotoStmt);
		CX_CURSOR_TOSTRING(ContinueStmt);
		CX_CURSOR_TOSTRING(BreakStmt);
		CX_CURSOR_TOSTRING(ReturnStmt);
		CX_CURSOR_TOSTRING(GCCAsmStmt);
		CX_CURSOR_TOSTRING(ObjCAtTryStmt);
		CX_CURSOR_TOSTRING(ObjCAtCatchStmt);
		CX_CURSOR_TOSTRING(ObjCAtFinallyStmt);
		CX_CURSOR_TOSTRING(ObjCAtThrowStmt);
		CX_CURSOR_TOSTRING(ObjCAtSynchronizedStmt);
		CX_CURSOR_TOSTRING(ObjCAutoreleasePoolStmt);
		CX_CURSOR_TOSTRING(ObjCForCollectionStmt);
		CX_CURSOR_TOSTRING(CXXCatchStmt);
		CX_CURSOR_TOSTRING(CXXTryStmt);
		CX_CURSOR_TOSTRING(CXXForRangeStmt);
		CX_CURSOR_TOSTRING(SEHTryStmt);
		CX_CURSOR_TOSTRING(SEHExceptStmt);
		CX_CURSOR_TOSTRING(SEHFinallyStmt);
		CX_CURSOR_TOSTRING(MSAsmStmt);
		CX_CURSOR_TOSTRING(NullStmt);
		CX_CURSOR_TOSTRING(DeclStmt);
		CX_CURSOR_TOSTRING(OMPParallelDirective);
		CX_CURSOR_TOSTRING(TranslationUnit);
		CX_CURSOR_TOSTRING(FirstAttr);
		CX_CURSOR_TOSTRING(IBActionAttr);
		CX_CURSOR_TOSTRING(IBOutletAttr);
		CX_CURSOR_TOSTRING(IBOutletCollectionAttr);
		CX_CURSOR_TOSTRING(CXXFinalAttr);
		CX_CURSOR_TOSTRING(CXXOverrideAttr);
		CX_CURSOR_TOSTRING(AnnotateAttr);
		CX_CURSOR_TOSTRING(AsmLabelAttr);
		CX_CURSOR_TOSTRING(PackedAttr);
		CX_CURSOR_TOSTRING(PreprocessingDirective);
		CX_CURSOR_TOSTRING(MacroDefinition);
		CX_CURSOR_TOSTRING(MacroExpansion);
		CX_CURSOR_TOSTRING(InclusionDirective);
		CX_CURSOR_TOSTRING(ModuleImportDecl);
#undef CX_CURSOR_TOSTRING
		default:
			return "Unknown CXCursorKind: " + std::to_string(kind);
		}
	}



	class UnsavedFile
	{
		CXUnsavedFile	_raw;

	public:
		UnsavedFile() { THROW(std::runtime_error("Not implemented")); }

		CXUnsavedFile GetRaw() const { return _raw; }
	};


	template < typename VisitorType_ >
	class VisitorBase
	{
	public:
		static CXChildVisitResult VisitorFunc(CXCursor cursor, CXCursor parent, CXClientData clientData);
	};

	BEGIN_CLANG_WRAPPER_NO_DISPOSE(Type)
		std::string GetSpelling() const { return StringWrapper(clang_getTypeSpelling(_raw)); }
	END_CLANG_WRAPPER();


	BEGIN_CLANG_WRAPPER_NO_DISPOSE(Module);
		std::string GetName() const { return StringWrapper(clang_Module_getName(_raw)); }
		std::string GetFullName() const { return StringWrapper(clang_Module_getFullName(_raw)); }
	END_CLANG_WRAPPER();


	BEGIN_CLANG_WRAPPER_NO_DISPOSE(File)
		std::string GetFileName() const { return StringWrapper(clang_getFileName(_raw)); }
		time_t GetFileTime() const { return clang_getFileTime(_raw); }
		CXFileUniqueID GetUniqueID() const { CXFileUniqueID res = { }; CHECK(clang_getFileUniqueID(_raw, &res) == 0, "clang_getFileUniqueID failed!"); return res; }
	END_CLANG_WRAPPER();


	BEGIN_CLANG_WRAPPER_NO_DISPOSE(SourceLocation)
		static SourceLocation GetNull() { return SourceLocation(clang_getNullLocation()); }

		File GetFile() const { CXFile res; clang_getExpansionLocation(_raw, &res, NULL, NULL, NULL); return res; }

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

		std::string GetSpelling() const { return StringWrapper(clang_getCursorSpelling(_raw)); }
		std::string GetDisplayName() const { return StringWrapper(clang_getCursorDisplayName(_raw)); }

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


	BEGIN_CLANG_WRAPPER(TranslationUnit, clang_disposeTranslationUnit)
		Cursor GetCursor() const { return clang_getTranslationUnitCursor(_raw); }
		Cursor GetCursor(const SourceLocation& loc) const { return clang_getCursor(_raw, loc.GetRaw()); }
		SourceLocation GetLocation(const File& file, size_t line, size_t column) const { return clang_getLocation(_raw, file.GetRaw(), line, column); }
		File GetFile(const std::string& filename) const { return clang_getFile(_raw, filename.c_str()); }
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

}

#endif
