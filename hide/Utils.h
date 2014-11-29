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

#define HIDE_DECLARE_TO_STRING_METHOD() \
	std::string ToString() const	{ return boost::lexical_cast<std::string>(*this); } \
	std::string __repr__() const	{ return ToString(); }

#define HIDE_DECLARE_WRITE_TO_OSTREAM(Type_, Code_) \
	inline std::ostream & operator<< (std::ostream & s, const Type_& v) \
	{ \
		Code_; \
		return s; \
	}

#endif
