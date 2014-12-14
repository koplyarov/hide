#ifndef HIDE_UTILS_CLASSCONTENTCHECKS_H
#define HIDE_UTILS_CLASSCONTENTCHECKS_H


#include <boost/mpl/if.hpp>
#include <boost/type_traits.hpp>

namespace hide
{

#define HIDE_DECLARE_NESTED_TYPE_CHECK(NestedType_) \
	template < typename T > \
	class HasNestedType_##NestedType_ \
	{ \
		template < typename U > static boost::type_traits::yes_type deduce(boost::mpl::int_<sizeof(typename boost::remove_reference<typename U::NestedType_>::type*)>*); \
		template < typename U > static boost::type_traits::no_type deduce(...); \
		\
	public: \
		static const bool Value = sizeof(deduce<T>(0)) == sizeof(boost::type_traits::yes_type); \
	}

	namespace Detail
	{
		struct DoesNotHaveAnyNestedTypes
		{ static const bool Value = false; };
	}

#define HIDE_DECLARE_METHOD_CHECK(Method_) \
	template < typename T > \
	struct HasMethod_##Method_ \
	{ \
		template <typename Type_> \
		class Impl \
		{ \
			struct BaseMixin { void Method_(){} }; \
			struct Base : public Type_, public BaseMixin { Base(); }; \
			\
			template <typename V, V t>    class Helper{}; \
			\
			template <typename U> static boost::type_traits::no_type deduce(U*, Helper<void (BaseMixin::*)(), &U::Method_>* = 0); \
			static boost::type_traits::yes_type deduce(...); \
			\
		public: \
			static const bool Value = (sizeof(boost::type_traits::yes_type) == sizeof(deduce((Base*)(0)))); \
		}; \
		static const bool Value = \
			boost::mpl::if_c< \
					boost::is_class<T>::value, \
					Impl<T>, \
					Detail::DoesNotHaveAnyNestedTypes \
				>::type::Value; \
	}

}

#endif
