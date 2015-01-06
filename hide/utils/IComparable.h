#ifndef HIDE_UTILS_ICOMPARABLE_H
#define HIDE_UTILS_ICOMPARABLE_H


#include <typeinfo>

#include <hide/utils/Utils.h>


namespace hide
{

	struct IComparable
	{
		virtual ~IComparable() { }

		virtual int Compare(const IComparable& other) const { HIDE_PURE_VIRTUAL_CALL(); }
	};
	HIDE_DECLARE_PTR(IComparable);


	template < typename FinalClass_ >
	class Comparable : public virtual IComparable
	{
	public:
		virtual int Compare(const IComparable& other) const
		{
			size_t my_hash = typeid(*this).hash_code();
			size_t other_hash = typeid(other).hash_code();

			if (my_hash != other_hash)
				return my_hash - other_hash;

			return DoCompare(dynamic_cast<const FinalClass_&>(other));
		}

	private:
		virtual int DoCompare(const FinalClass_& other) const = 0;
	};

}

#endif
