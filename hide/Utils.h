#ifndef UTILS_H
#define UTILS_H

#include <memory>
#include <stdexcept>

#include <boost/exception/all.hpp>

#define HIDE_DECLARE_PTR(T_) typedef std::shared_ptr<T_>	T_##Ptr
#define HIDE_CHECK(Expr_, Exception_) do { if (!(Expr_)) BOOST_THROW_EXCEPTION(Exception_); } while (false)

template < typename T > T RequireNotNull(const T& val, const char* msg) { HIDE_CHECK(val, std::runtime_error(msg)); return val; }

#define REQUIRE_NOT_NULL(Val_) RequireNotNull(Val_, "Null value: " #Val_)

#define HIDE_NONCOPYABLE(Class_) \
	private: \
	Class_(const Class_&); \
	Class_& operator=(const Class_&);

#endif
