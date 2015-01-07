#ifndef HIDE_UTILS_MEMBERSVISITOR_H
#define HIDE_UTILS_MEMBERSVISITOR_H


#include <string>


namespace hide
{

#ifndef SWIG

	namespace Detail {
	namespace MembersVisitor
	{

		template < typename Visitor, typename ClassRef >
		void DoVisitMembers(Visitor&& v, ClassRef&& inst)
		{ }

		template < typename Visitor, typename ClassRef, typename C, typename M, typename... Tail >
		void DoVisitMembers(Visitor&& v, ClassRef&& inst, const std::string& memberName, M C::*member, const Tail&... tail)
		{
			v.Visit(inst, memberName, member);
			DoVisitMembers(std::forward<Visitor>(v), std::forward<ClassRef>(inst), tail...);
		}

	}}

#	define HIDE_DECLARE_MEMBERS(...) \
		template < typename Visitor > void VisitMembers(Visitor&& v) { ::hide::Detail::MembersVisitor::DoVisitMembers(v, *this, __VA_ARGS__); } \
		template < typename Visitor > void VisitMembers(Visitor&& v) const { ::hide::Detail::MembersVisitor::DoVisitMembers(v, *this, __VA_ARGS__); }
#else
#	define HIDE_DECLARE_MEMBERS(...)
#endif

}

#endif
