#ifndef CLANG_STRING_H
#define CLANG_STRING_H


#include <hide/utils/Utils.h>
#include <hide/lang_plugins/cpp/clang/Common.h>


namespace hide {
namespace cpp {
namespace clang
{

	class String
	{
		CXString	_raw;

	public:
		String(CXString raw) : _raw(raw) { REQUIRE_NOT_NULL(_raw.data); }
		~String() { clang_disposeString(_raw); }
		operator std::string() const { return clang_getCString(_raw); }
	};

}}}

#endif
