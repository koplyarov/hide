#ifndef HIDE_UTILS_STRINGBUILDER_H
#define HIDE_UTILS_STRINGBUILDER_H


#include <memory>
#include <sstream>
#include <utility>

#include <boost/exception/diagnostic_information.hpp>
#include <boost/filesystem/path.hpp>

#include <hide/utils/ClassContentChecks.h>
#include <hide/utils/Utils.h>


namespace hide
{

	namespace Detail {
	namespace StringBuilder
	{
		struct ObjectType
		{
			HIDE_ENUM_VALUES(HasToString, HasVisitMembers, Collection, Exception, Other);
			HIDE_ENUM_CLASS(ObjectType);
		};

		HIDE_DECLARE_METHOD_CHECK(ToString);
		HIDE_DECLARE_METHOD_CHECK(VisitMembers);
		HIDE_DECLARE_METHOD_CHECK(begin);
		HIDE_DECLARE_METHOD_CHECK(end);

		template < typename T >
		struct ObjectTypeGetter
		{
			static const ObjectType::Enum Value =
				HasMethod_ToString<T>::Value ? ObjectType::HasToString :
					(HasMethod_VisitMembers<T>::Value ? ObjectType::HasVisitMembers :
						(HasMethod_begin<T>::Value && HasMethod_end<T>::Value ? ObjectType::Collection :
							(std::is_base_of<std::exception, T>::value ? ObjectType::Exception :
								ObjectType::Other)));
		};

		template < typename T, ObjectType::Enum ObjType_ = ObjectTypeGetter<T>::Value >
		struct Writer
		{
			static void Write(std::stringstream& s, const T& val)
			{ s << val; }
		};

		template < typename T >
		void WriteToStream(std::stringstream& s, const T& val)
		{ Writer<T>::Write(s, val); }

		template < typename T >
		struct Writer<T, ObjectType::HasToString>
		{
			static void Write(std::stringstream& s, const T& val)
			{ s << val.ToString(); }
		};

		class ToStringVisitor
		{
		private:
			bool&					_first;
			std::stringstream&		_stream;

		public:
			ToStringVisitor(bool& first, std::stringstream& stream)
				: _first(first), _stream(stream)
			{ }

			template < typename ClassRef, typename C, typename M >
			void Visit(ClassRef&& inst, const std::string& memberName, M C::*member)
			{
				_stream << (_first ? " " : ", ") << memberName << ": ";
				WriteToStream(_stream, inst.*member);
				_first = false;
			}
		};

		template < typename T >
		struct Writer<T, ObjectType::HasVisitMembers>
		{
			static void Write(std::stringstream& s, const T& val)
			{
				s << "{";
				bool first = true;
				val.VisitMembers(ToStringVisitor(first, s));
				s << " }";
			}
		};

		template < >
		struct Writer<std::string, ObjectType::Collection>
		{
			static void Write(std::stringstream& s, const std::string& val)
			{ s << val; }
		};

		template < >
		struct Writer<boost::filesystem::path, ObjectType::Collection>
		{
			static void Write(std::stringstream& s, const boost::filesystem::path& val)
			{ s << val; }
		};

		template < typename T >
		struct Writer<T, ObjectType::Collection>
		{
			static void Write(std::stringstream& s, const T& val)
			{
				s << "[";
				bool first = true;
				for (auto e : val)
				{
					s << (first ? " " : ", ");
					WriteToStream(s, e);
					first = false;
				}
				s << " ]";
			}
		};

		template < typename T >
		struct Writer<T, ObjectType::Exception>
		{
			static void Write(std::stringstream& s, const T& val)
			{ s << boost::diagnostic_information(val); }
		};


		template < typename K, typename V >
		struct Writer<std::pair<K, V>, ObjectType::Other>
		{
			static void Write(std::stringstream& s, const std::pair<K, V>& val)
			{
				s << "(";
				WriteToStream(s, val.first);
				s << ", ";
				WriteToStream(s, val.second);
				s << ")";
			}
		};

		template < typename T >
		struct Writer<std::shared_ptr<T>, ObjectType::Other>
		{
			static void Write(std::stringstream& s, const std::shared_ptr<T>& val)
			{
				if (val)
					WriteToStream(s, *val);
				else
					s << "null";
			}
		};
	}}

	class StringBuilder
	{
	private:
		std::stringstream		_stream;

	public:
		template < typename T >
		StringBuilder& operator % (const T& val)
		{
			Detail::StringBuilder::WriteToStream(_stream, val);
			return *this;
		}

		std::string ToString() const
		{ return _stream.str(); }

		operator std::string() const
		{ return ToString(); }
	};


	template < typename T >
	class HexValue
	{
	private:
		T		_value;

	public:
		HexValue(T value) : _value(value) { }

		std::string ToString() const { return StringBuilder() % "0x" % std::hex % _value; }
	};

	template < typename T >
	HexValue<T> Hex(T value)
	{ return HexValue<T>(value); }
}

#endif
