%module hide

%{
#include <hide/Project.h>
using namespace hide;
%}

%include <std_shared_ptr.i>
%include <std_string.i>

%include <hide/Utils.h>

%shared_ptr(hide::Buffer)
%include <hide/Buffer.h>

%include <hide/Project.h>
