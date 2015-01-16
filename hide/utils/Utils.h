#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

#include <chrono>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/exception/all.hpp>
#include <boost/preprocessor.hpp>

namespace hide
{

#define HIDE_DECLARE_PTR(T_) typedef std::shared_ptr<T_>	T_##Ptr
#define HIDE_DECLARE_ARRAY(T_) typedef std::vector<T_>	T_##Array
#define HIDE_DECLARE_MAP(K_, V_) typedef std::map<K_, V_>	K_##To##V_##Map
#define HIDE_CHECK(Expr_, Exception_) do { if (!(Expr_)) BOOST_THROW_EXCEPTION(Exception_); } while (false)
#define HIDE_LOCK(Mutex_)	std::lock_guard<decltype(Mutex_)> BOOST_PP_CAT(lock, __LINE__)(Mutex_);

	typedef std::time_t	Time;

	typedef std::string String;
	HIDE_DECLARE_ARRAY(String);

	typedef char Byte;
	HIDE_DECLARE_ARRAY(Byte);

	template < typename T > T RequireNotNull(const T& val, const char* msg) { HIDE_CHECK(val, std::runtime_error(msg)); return val; }

#define REQUIRE_NOT_NULL(Val_) ::hide::RequireNotNull(Val_, "Null value: " #Val_)

#define HIDE_NONCOPYABLE(Class_) \
	private: \
	Class_(const Class_&); \
	Class_& operator=(const Class_&);

#define HIDE_DECLARE_SWIG_TO_STRING_WRAPPER() \
	std::string __repr__() const	{ return StringBuilder() % *this; }

#define HIDE_DECLARE_TO_STRING_METHOD() \
	std::string ToString() const	{ return boost::lexical_cast<std::string>(*this); } \
	std::string __repr__() const	{ return ToString(); }

#define HIDE_DECLARE_WRITE_TO_OSTREAM(Type_, Code_) \
	inline std::ostream & operator<< (std::ostream & s, const Type_& v) \
	{ \
		Code_; \
		return s; \
	}

#define DETAIL_HIDE_ENUM_TO_STRING_CASE(R_, Data_, Elem_) case Elem_: return BOOST_PP_STRINGIZE(Elem_);
#define DETAIL_HIDE_ENUM_FROM_STRING_IF(R_, Data_, Elem_) if (str == BOOST_PP_STRINGIZE(Elem_)) return Elem_;
#define HIDE_ENUM_VALUES(...) \
	public: \
		enum Enum { __VA_ARGS__ }; \
	private: \
		static Enum DoGetFirstVal(Enum first, ...) { return first; } \
		static Enum GetFirstVal() { return DoGetFirstVal(__VA_ARGS__); } \
	public: \
		std::string ToString() const \
		{ \
			switch (_val) \
			{ \
				BOOST_PP_SEQ_FOR_EACH(DETAIL_HIDE_ENUM_TO_STRING_CASE, ~, BOOST_PP_TUPLE_TO_SEQ((__VA_ARGS__))) \
			default: return GetClassName() + "(" + std::to_string(_val) + ")"; \
			} \
		} \
		static Enum FromString(const std::string& str) \
		{ \
			BOOST_PP_SEQ_FOR_EACH(DETAIL_HIDE_ENUM_FROM_STRING_IF, ~, BOOST_PP_TUPLE_TO_SEQ((__VA_ARGS__))) \
			BOOST_THROW_EXCEPTION(std::runtime_error("Could not parse " + GetClassName() + " value: " + str)); \
		}

#define HIDE_ENUM_CLASS(Class_) \
	private: \
		Enum _val; \
		static std::string GetClassName() { return #Class_; } \
	public: \
		Class_() : _val(GetFirstVal()) { } \
		Class_(Enum val) : _val(val) { } \
		operator Enum () const { return GetRaw(); } \
		Enum GetRaw() const { return _val; }


	class PureVirtualCallException : public std::runtime_error
	{
	public:
		PureVirtualCallException(const std::string& func)
			: std::runtime_error("Pure virtual call: " + func)
		{ }

		virtual ~PureVirtualCallException() throw()
		{ }
	};

#define HIDE_PURE_VIRTUAL_CALL() BOOST_THROW_EXCEPTION(PureVirtualCallException(__func__));

}

#endif
