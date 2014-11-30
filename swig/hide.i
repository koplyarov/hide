%module hide

%include <std_map.i>
%include <std_shared_ptr.i>
%include <std_string.i>
%include <std_vector.i>

%{
#include <hide/Location.h>
#include <hide/Project.h>
#include <hide/buildsystems/cmake/CMakeBuildConfig.h>
using namespace hide;
%}

%template(StringVector) std::vector<std::string>;

%include <hide/utils/Utils.h>

%include <hide/Location.h>

%shared_ptr(hide::IFile)
%include <hide/IFile.h>

%template(FileVector) std::vector<std::shared_ptr<hide::IFile> >;

%shared_ptr(hide::IBuildSystem)
%shared_ptr(hide::IBuildConfig)
%include <hide/IBuildSystem.h>

%template(StringToBuildConfigMap) std::map<std::string, std::shared_ptr<hide::IBuildConfig> >;

%shared_ptr(hide::CMakeBuildConfig)
%include <hide/buildsystems/cmake/CMakeBuildConfig.h>

%shared_ptr(hide::Buffer)
%include <hide/Buffer.h>

%shared_ptr(hide::Project)
%include <hide/Project.h>
