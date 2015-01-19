#include <hide/lang_plugins/GenericCTagsIndexer.h>

#include <fstream>
#include <future>
#include <sstream>

#include <boost/algorithm/string.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/regex.hpp>

#include <hide/lang_plugins/CTagsInvoker.h>
#include <hide/utils/Comparers.h>
#include <hide/utils/Executable.h>
#include <hide/utils/MembersVisitor.h>
#include <hide/utils/PTree.h>
#include <hide/utils/ReadBufferLinesListener.h>


namespace hide
{


	class CTagsIndexEntry : public Comparable<CTagsIndexEntry>, public virtual IIndexEntry
	{
	public:
		struct SerializationProxy
		{
			std::string		Name;
			std::string		Scope;
			IndexEntryKind	Kind;
			int				Line;

			SerializationProxy() : Line(0) { }
			SerializationProxy(const CTagsIndexEntry& e) : Name(e._name), Scope(e._scope), Kind(e._kind), Line(e._location.GetLine()) { }

			CTagsIndexEntry ToEntry(const std::string& filename) const { return CTagsIndexEntry(Name, Scope, Kind, Location(filename, Line, 1)); }

			HIDE_DECLARE_MEMBERS("name", &HIDE_SELF_TYPE::Name, "scope", &HIDE_SELF_TYPE::Scope, "kind", &HIDE_SELF_TYPE::Kind, "line", &HIDE_SELF_TYPE::Line);
		};

	private:
		std::string		_name;
		std::string		_scope;
		IndexEntryKind	_kind;
		Location		_location;

	public:
		CTagsIndexEntry()
		{ }

		CTagsIndexEntry(const std::string& name, const std::string& scope, IndexEntryKind kind, const Location& location)
			: _name(name), _scope(scope), _kind(kind), _location(location)
		{ }

		virtual std::string GetName() const		{ return _name; }
		virtual std::string GetScope() const	{ return _scope; }
		virtual std::string GetFullName() const	{ return _scope.empty() ? _name : _scope + "::" + _name; }
		virtual IndexEntryKind GetKind() const	{ return _kind; }
		virtual Location GetLocation() const	{ return _location; }

		HIDE_DECLARE_MEMBERS("name", &CTagsIndexEntry::_name, "scope", &CTagsIndexEntry::_scope, "kind", &CTagsIndexEntry::_kind, "location", &CTagsIndexEntry::_location)

	protected:
		virtual int DoCompare(const CTagsIndexEntry& other) const
		{ return Cmp()(GetFullName(), other.GetFullName()); } // TODO: compare member lists
	};


	class CTagsPartialIndex : public virtual IPartialIndex
	{
	private:
		IIndexEntryPtrArray		_entries;

	public:
		CTagsPartialIndex(const IIndexEntryPtrArray& entries)
			: _entries(entries)
		{ }

		virtual void Save(const std::string& filename)
		{
			using namespace boost::property_tree;

			std::vector<CTagsIndexEntry::SerializationProxy> ctags_entries;
			std::transform(_entries.begin(), _entries.end(), std::back_inserter(ctags_entries), [](const IIndexEntryPtr& e) { return dynamic_cast<const CTagsIndexEntry&>(*e); });

			ptree root;
			PTreeWriter w(root);
			w.Write("entries", ctags_entries);

			std::ofstream f(filename, std::ios_base::trunc);
			write_json(f, root, false);
		}

		virtual IIndexEntryPtrArray GetEntries() { return _entries; }
	};


	HIDE_NAMED_LOGGER(GenericCTagsIndexer);

	GenericCTagsIndexer::GenericCTagsIndexer(const std::string& filename)
		: _filename(filename)
	{ }


	IPartialIndexPtr GenericCTagsIndexer::BuildIndex()
	{
		IIndexEntryPtrArray entries;

		StringArray scope_fields = { "class", "struct", "namespace" };
		std::map<std::string, IndexEntryKind> kinds_map = { { "class", IndexEntryKind::Type }, { "struct", IndexEntryKind::Type }, { "member", IndexEntryKind::Variable }, { "function", IndexEntryKind::Function } };

		CTagsInvoker({ "-f-", "--excmd=number", "--sort=no", "--fields=+aimSztK", _filename },
				[&](const std::string& name, int line, const CTagsOutputParser::FieldsMap& fields)
				{
					std::string scope;
					for (const auto& scope_field : scope_fields)
					{
						auto f_it = fields.find(scope_field);
						if (f_it != fields.end())
						{
							scope = f_it->second;
							break;
						}
					}

					std::string kind_str = fields.at("kind");
					auto k_it = kinds_map.find(fields.at("kind"));
					IndexEntryKind kind = k_it != kinds_map.end() ? k_it->second : IndexEntryKind();

					entries.push_back(std::make_shared<CTagsIndexEntry>(name, scope, kind, Location(_filename, line, 1)));
				}
			);

		return std::make_shared<CTagsPartialIndex>(entries);
	}


	IPartialIndexPtr GenericCTagsIndexer::LoadIndex(const std::string& filename)
	{
		using namespace boost::property_tree;

		ptree root;
		std::ifstream f(filename);
		read_json(f, root);

		PTreeReader wr(root);
		std::vector<CTagsIndexEntry::SerializationProxy> ctags_entries;
		PTreeReader(root).Read("entries", ctags_entries);

		std::vector<IIndexEntryPtr> entries;
		for (const auto& e : ctags_entries)
			entries.push_back(std::make_shared<CTagsIndexEntry>(e.ToEntry(_filename)));

		return std::make_shared<CTagsPartialIndex>(entries);
	}

}
