#ifndef HIDE_UTILS_PTREE_H
#define HIDE_UTILS_PTREE_H


#include <boost/property_tree/ptree.hpp>


namespace hide
{

	namespace Detail {
	namespace PTree
	{
		struct ObjectType
		{
			HIDE_ENUM_VALUES( HasReadWrite, HasVisitMembers, HasToAndFromString, Collection, Other);
			HIDE_ENUM_CLASS(ObjectType);
		};

		HIDE_DECLARE_METHOD_CHECK(VisitMembers);
		HIDE_DECLARE_METHOD_CHECK(WriteToPTree);
		HIDE_DECLARE_METHOD_CHECK(ReadFromPTree);
		HIDE_DECLARE_METHOD_CHECK(ToString);
		HIDE_DECLARE_METHOD_CHECK(FromString);
		HIDE_DECLARE_METHOD_CHECK(begin);
		HIDE_DECLARE_METHOD_CHECK(end);

		template < typename T >
		struct ObjectTypeGetter
		{
			static const ObjectType::Enum Value =
				HasMethod_WriteToPTree<T>::Value && HasMethod_ReadFromPTree<T>::Value ? ObjectType::HasReadWrite :
					(HasMethod_VisitMembers<T>::Value ? ObjectType::HasVisitMembers :
						(HasMethod_ToString<T>::Value && HasMethod_FromString<T>::Value ? ObjectType::HasToAndFromString :
							(HasMethod_begin<T>::Value && HasMethod_end<T>::Value ? ObjectType::Collection :
								ObjectType::Other)));
		};


		template < typename T, ObjectType::Enum ObjType_ = ObjectTypeGetter<T>::Value >
		struct Writer
		{
			static void Write(boost::property_tree::ptree& t, const std::string& name, const T& val)
			{ t.put<T>(name, val); }
		};

		template < typename T, ObjectType::Enum ObjType_ = ObjectTypeGetter<T>::Value >
		struct Reader
		{
			static void Read(const boost::property_tree::ptree& t, const std::string& name, T& val)
			{ val = t.get<T>(name); }
		};

		template < typename T >
		void WriteToPTree(boost::property_tree::ptree& t, const std::string& name, const T& val)
		{ Writer<T>::Write(t, name, val); }

		template < typename T >
		void ReadFromPTree(const boost::property_tree::ptree& t, const std::string& name, T& val)
		{ Reader<T>::Read(t, name, val); }

		template < typename T >
		struct Writer<T, ObjectType::HasReadWrite>
		{
			static void Write(boost::property_tree::ptree& t, const std::string& name, const T& val)
			{
				boost::property_tree::ptree child;
				val.WriteToPTree(child);
				t.push_back(std::make_pair(name, child));
			}
		};

		template < typename T >
		struct Reader<T, ObjectType::HasReadWrite>
		{
			static void Read(const boost::property_tree::ptree& t, const std::string& name, T& val)
			{ val.ReadFromPTree(t.get_child(name)); }
		};

		template < typename T >
		struct Writer<T, ObjectType::HasToAndFromString>
		{
			static void Write(boost::property_tree::ptree& t, const std::string& name, const T& val)
			{ WriteToPTree(t, name, val.ToString()); }
		};

		template < typename T >
		struct Reader<T, ObjectType::HasToAndFromString>
		{
			static void Read(const boost::property_tree::ptree& t, const std::string& name, T& val)
			{
				std::string s;
				ReadFromPTree(t, name, s);
				val = T::FromString(s);
			}
		};

		class PTreeWriteVisitor
		{
		private:
			boost::property_tree::ptree&		_node;

		public:
			PTreeWriteVisitor(boost::property_tree::ptree& node)
				: _node(node)
			{ }

			template < typename ClassRef, typename C, typename M >
			void Visit(ClassRef&& inst, const std::string& memberName, M C::*member)
			{ WriteToPTree(_node, memberName, inst.*member); }
		};

		template < typename T >
		struct Writer<T, ObjectType::HasVisitMembers>
		{
			static void Write(boost::property_tree::ptree& t, const std::string& name, const T& val)
			{
				boost::property_tree::ptree child;
				val.VisitMembers(PTreeWriteVisitor(child));
				t.push_back(std::make_pair(name, child));
			}
		};

		class PTreeReadVisitor
		{
		private:
			const boost::property_tree::ptree&		_node;

		public:
			PTreeReadVisitor(const boost::property_tree::ptree& node)
				: _node(node)
			{ }

			template < typename ClassRef, typename C, typename M >
			void Visit(ClassRef&& inst, const std::string& memberName, M C::*member)
			{ ReadFromPTree(_node, memberName, inst.*member); }
		};

		template < typename T >
		struct Reader<T, ObjectType::HasVisitMembers>
		{
			static void Read(const boost::property_tree::ptree& t, const std::string& name, T& val)
			{ val.VisitMembers(PTreeReadVisitor(t.get_child(name))); }
		};

		template < >
		struct Writer<std::string, ObjectType::Collection>
		{
			static void Write(boost::property_tree::ptree& t, const std::string& name, const std::string& val)
			{ t.put<std::string>(name, val); }
		};

		template < >
		struct Reader<std::string, ObjectType::Collection>
		{
			static void Read(const boost::property_tree::ptree& t, const std::string& name, std::string& val)
			{ val = t.get<std::string>(name); }
		};

		template < typename T >
		struct Writer<T, ObjectType::Collection>
		{
			static void Write(boost::property_tree::ptree& t, const std::string& name, const T& val)
			{
				boost::property_tree::ptree child;
				for (const auto& e : val)
					WriteToPTree(child, "", e);
				t.push_back(std::make_pair(name, child));
			}
		};

		template < typename T >
		struct Reader<T, ObjectType::Collection>
		{
			static void Read(const boost::property_tree::ptree& t, const std::string& name, T& val)
			{
				for (const auto& e : t.get_child(name))
				{
					typename T::value_type array_element;
					ReadFromPTree(e.second, "", array_element);
					val.push_back(array_element);
				}
			}
		};

	}}


	class PTreeWriter
	{
	private:
		boost::property_tree::ptree&		_node;

	public:
		PTreeWriter(boost::property_tree::ptree& node)
			: _node(node)
		{ }

		template < typename T >
		PTreeWriter& Write(const std::string& name, const T& val)
		{
			Detail::PTree::WriteToPTree(_node, name, val);
			return *this;
		}
	};


	class PTreeReader
	{
	private:
		const boost::property_tree::ptree&	_node;

	public:
		PTreeReader(const boost::property_tree::ptree& node)
			: _node(node)
		{ }

		template < typename T >
		PTreeReader& Read(const std::string& name, T& val)
		{
			Detail::PTree::ReadFromPTree(_node, name, val);
			return *this;
		}
	};


}

#endif
