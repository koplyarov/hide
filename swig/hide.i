%module hide

%include <std_shared_ptr.i>
%include <std_string.i>
%include <std_vector.i>

%{
#include <hide/Location.h>
#include <hide/Project.h>
using namespace hide;
%}

%template(StringVector) std::vector<std::string>;

%include <hide/Utils.h>

%include <hide/Location.h>

%shared_ptr(hide::IFile)
%include <hide/IFile.h>

%template(FileVector) std::vector<std::shared_ptr<hide::IFile> >;

%shared_ptr(hide::Buffer)
%include <hide/Buffer.h>

%shared_ptr(hide::Project)
%include <hide/Project.h>
