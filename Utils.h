#ifndef UTILS_H
#define UTILS_H

#include <memory>
#include <stdexcept>

#define DECLARE_PTR(T_) typedef std::shared_ptr<T_>	T_##Ptr
#define THROW(Exception_) throw Exception_
#define CHECK(Expr_, Exception_) do { if (!(Expr_)) THROW(Exception_); } while (false)

template < typename T > T RequireNotNull(const T& val, const char* msg) { CHECK(val, std::runtime_error(msg)); return val; }

#define REQUIRE_NOT_NULL(Val_) RequireNotNull(Val_, "Null value: " #Val_)

#endif
