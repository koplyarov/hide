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

%implicitconv hide::LogLevel;
%include <hide/utils/LoggerMessage.h>

%feature("director") hide::ILoggerSink;
%shared_ptr(hide::ILoggerSink)
%include <hide/utils/ILoggerSink.h>

%include <hide/utils/Logger.h>

%include <hide/Location.h>

%feature("director") hide::IBuildLogListener;
%shared_ptr(hide::BuildIssue)
%shared_ptr(hide::IBuildLogListener)
%shared_ptr(hide::BuildLog)
%shared_ptr(hide::BuildLogControl)
%include <hide/BuildLog.h>

%shared_ptr(hide::IFile)
%include <hide/IFile.h>

%template(FileVector) std::vector<std::shared_ptr<hide::IFile> >;

%shared_ptr(hide::IBuildSystem)
%shared_ptr(hide::IBuildConfig)
%shared_ptr(hide::IBuildProcess)
%include <hide/IBuildSystem.h>

%template(StringToBuildConfigMap) std::map<std::string, std::shared_ptr<hide::IBuildConfig> >;

%shared_ptr(hide::CMakeBuildConfig)
%include <hide/buildsystems/cmake/CMakeBuildConfig.h>

%shared_ptr(hide::Buffer)
%include <hide/Buffer.h>

%shared_ptr(hide::Project)
%include <hide/Project.h>
