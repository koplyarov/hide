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
		// TODO: get the exception message
		throw std::runtime_error(std::string(Swig::DirectorMethodException().getMessage()) );
	}
}

%{
#include <hide/Location.h>
#include <hide/Project.h>
#include <hide/buildsystems/cmake/CMakeBuildConfig.h>
using namespace hide;
%}

%template(StringVector) std::vector<std::string>;

%ignore operator Enum;

%include <hide/utils/Utils.h>

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

%shared_ptr(hide::Indexer)
%include <hide/Indexer.h>

%shared_ptr(hide::Buffer)
%include <hide/Buffer.h>

%shared_ptr(hide::Project)
%include <hide/Project.h>
