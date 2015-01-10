#ifndef HIDE_UTILS_COMPARERS_H
#define HIDE_UTILS_COMPARERS_H


#include <hide/utils/ClassContentChecks.h>
#include <hide/utils/IComparable.h>
#include <type_traits>


namespace hide
{

	namespace Detail {
	namespace Comparers
	{

		struct ObjectType
		{
			HIDE_ENUM_VALUES(IComparable, Collection, Other);
			HIDE_ENUM_CLASS(ObjectType);
		};

		HIDE_DECLARE_METHOD_CHECK(begin);
		HIDE_DECLARE_METHOD_CHECK(end);


		template < typename T >
		struct ObjectTypeGetter
		{
			static const ObjectType::Enum Value =
				std::is_base_of<IComparable, T>::value ? ObjectType::IComparable :
					(HasMethod_begin<T>::Value && HasMethod_end<T>::Value ? ObjectType::Collection :
						ObjectType::Other);
		};


		template < typename T, ObjectType::Enum ObjType_ = ObjectTypeGetter<T>::Value >
		struct Comparer
		{
			static int Compare(const T& l, const T& r)
			{ return l < r ? -1 : (r < l ? 1 : 0); }
		};

		template < typename T >
		int InvokeCompare(const T& l, const T& r)
		{ return Comparer<T>::Compare(l, r); }

		template < typename T >
		struct Comparer<T, ObjectType::IComparable>
		{
			static int Compare(const T& l, const T& r)
			{ return l.Compare(r); }
		};

		template < typename T >
		struct Comparer<std::shared_ptr<T>, ObjectType::Other>
		{
			static int Compare(const std::shared_ptr<T>& l, const std::shared_ptr<T>& r)
			{
				if (!l || !r)
					return (!l && !r) ? 0 : (r ? -1 : 1);
				return InvokeCompare(*l, *r);
			}
		};

	}}


	struct Less
	{
		template < typename T >
		bool operator () (const T& l, const T& r) const
		{ return Detail::Comparers::InvokeCompare(l, r) < 0; }
	};

}

#endif
