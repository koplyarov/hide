%module(directors="1") hide

%include <exception.i>
%include <std_map.i>
%include <std_shared_ptr.i>
%include <std_string.i>
%include <std_vector.i>

%exception {
	try { $action }
	catch (const std::exception& e) { SWIG_exception(SWIG_RuntimeError, e.what()); }
}

%feature("director:except") {
	if ($error != NULL) {
		struct PyObjectHolder
		{
			PyObject*	Obj;

			PyObjectHolder(PyObject* obj) : Obj(obj) { }
			~PyObjectHolder() { Py_DECREF(Obj); }
		};

		std::string msg(Swig::DirectorMethodException().getMessage());
		PyObject *type = NULL, *value = NULL, *traceback = NULL;
		PyErr_Fetch(&type, &value, &traceback);
		if (value)
		{
			PyObjectHolder value_str(PyObject_Str(value));
			const char* err_str = PyString_AsString(value_str.Obj);
			if (err_str)
				msg += std::string(" (") + err_str + ")";
		}
		PyErr_Restore(type, value, traceback);
		throw std::runtime_error(msg);
	}
}

%{
#include <hide/Location.h>
#include <hide/Project.h>
#include <hide/buildsystems/cmake/CMakeBuildConfig.h>
%}

%template(StringVector) std::vector<std::string>;

%ignore operator Enum;

%include <hide/utils/Utils.h>
%include <hide/utils/MembersVisitor.h>
%include <hide/utils/Diff.h>

%ignore hide::MakeThread;
%include <hide/utils/Thread.h>

%implicitconv hide::LogLevel;
%copyctor hide::LoggerMessage;
%include <hide/utils/LoggerMessage.h>

%feature("director") hide::ILoggerSink;
%shared_ptr(hide::ILoggerSink)
%include <hide/utils/ILoggerSink.h>

%include <hide/utils/Logger.h>

%warnfilter(325) hide::NamedLogger::StreamAccessProxy;
%ignore hide::NamedLogger::StreamAccessProxy;
%ignore hide::NamedLogger::Debug() const;
%ignore hide::NamedLogger::Info() const;
%ignore hide::NamedLogger::Warning() const;
%ignore hide::NamedLogger::Error() const;
%include <hide/utils/NamedLogger.h>

%include <hide/Location.h>

%shared_ptr(hide::BuildIssue)
%copyctor hide::BuildLogLine;
%include <hide/BuildLogLine.h>

%feature("director") hide::IComparable;
%shared_ptr(hide::IComparable)
%include <hide/utils/IComparable.h>

%feature("director") hide::IPartialIndex;
%shared_ptr(hide::IIndexEntry)
%template(IIndexEntryVector) std::vector<std::shared_ptr<hide::IIndexEntry> >;
%template(IIndexEntryDiff) hide::Diff<std::shared_ptr<hide::IIndexEntry> >;
%shared_ptr(hide::IPartialIndex)
%include <hide/IPartialIndex.h>

%feature("director") hide::IPartialIndexer;
%shared_ptr(hide::IPartialIndexer)
%include <hide/IPartialIndexer.h>

%feature("director") hide::IIndexableId;
%shared_ptr(hide::IIndexableId)
%feature("director") hide::IIndexable;
%shared_ptr(hide::IIndexable)
%include <hide/IIndexable.h>

%feature("director") hide::IFile;
%shared_ptr(hide::IFile)
%include <hide/IFile.h>

%template(FileVector) std::vector<std::shared_ptr<hide::IFile> >;

%feature("director") hide::IBuildProcessListener;
%shared_ptr(hide::IBuildProcessListener)
%shared_ptr(hide::IBuildSystem)
%shared_ptr(hide::IBuildConfig)
%shared_ptr(hide::IBuildProcess)
%include <hide/IBuildSystem.h>

%template(StringToBuildConfigMap) std::map<std::string, std::shared_ptr<hide::IBuildConfig> >;

%shared_ptr(hide::CMakeBuildConfig)
%include <hide/buildsystems/cmake/CMakeBuildConfig.h>

%feature("director") hide::IIndexQueryListener;
%copyctor hide::IndexQueryEntry;
%shared_ptr(hide::IIndexQueryListener)
%shared_ptr(hide::IIndexQuery)
%include <hide/IIndexQuery.h>

%feature("director") hide::IIndexerListener;
%shared_ptr(hide::IIndexerListener)
%shared_ptr(hide::Indexer)
%ignore hide::Indexer::Indexer;
%include <hide/Indexer.h>

%feature("director") hide::IContextUnawareSyntaxHighlighterListener;
%shared_ptr(hide::IContextUnawareSyntaxHighlighterListener)
%shared_ptr(hide::ContextUnawareSyntaxHighlighter)
%copyctor hide::SyntaxWordCategory;
%template(SyntaxWordInfoArray) std::vector<hide::SyntaxWordInfo>;
%template(SyntaxWordInfoDiff) hide::Diff<hide::SyntaxWordInfo>;
%include <hide/ContextUnawareSyntaxHighlighter.h>

%shared_ptr(hide::Buffer)
%include <hide/Buffer.h>

%shared_ptr(hide::Project)
%include <hide/Project.h>
