#ifndef CLANG_COMMON_H
#define CLANG_COMMON_H


#include <clang-c/CXCompilationDatabase.h>
#include <clang-c/Index.h>


namespace hide {
namespace cpp {
namespace clang
{

#define BEGIN_CLANG_WRAPPER_NO_DISPOSE(Name_) \
	class Name_; \
	HIDE_DECLARE_PTR(Name_); \
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


}}}

#endif
