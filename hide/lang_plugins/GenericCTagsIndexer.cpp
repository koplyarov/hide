#include <hide/lang_plugins/GenericCTagsIndexer.h>

#include <fstream>
#include <future>
#include <sstream>

#include <boost/algorithm/string.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/regex.hpp>

#include <hide/utils/Comparers.h>
#include <hide/utils/Executable.h>
#include <hide/utils/MembersVisitor.h>
#include <hide/utils/PTree.h>
#include <hide/utils/ReadBufferLinesListener.h>


namespace hide
{


	class CTagsIndexEntry : public Comparable<CTagsIndexEntry>, public virtual IIndexEntry
	{
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

		void WriteToPTree(boost::property_tree::ptree& node) const
		{
			PTreeWriter w(node);
			w.Write("name", _name).Write("scope", _scope).Write("kind", _kind).Write("line", _location.GetLine());
		}

		void ReadFromPTree(const boost::property_tree::ptree& node)
		{
			PTreeReader r(node);
			size_t line;
			r.Read("name", _name).Read("scope", _scope).Read("kind", _kind).Read("line", line);
			_location = Location("", line, 1);
		}

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

			std::vector<CTagsIndexEntry> ctags_entries;
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
		using namespace boost;

		typedef std::map<std::string, std::string> FieldsMap;

		StringArray params;
		params.push_back("-f-");
		params.push_back("--excmd=number");
		params.push_back("--sort=no");
		params.push_back("--fields=+aimSztK");
		params.push_back(_filename);

		smatch m;
		regex re("^([^	]+)	([^	]+)	(\\d+);\"	(.*)$");

		IIndexEntryPtrArray entries;

		ExecutablePtr ctags = std::make_shared<Executable>("ctags", params);
		std::promise<void> stdout_closed;
		ctags->GetStdout()->AddListener(std::make_shared<ReadBufferLinesListener>(
				[&](const std::string& s)
				{
					if (regex_match(s, m, re))
					{
						std::string name = m[1];
						size_t line = std::stoi(m[3]);
						FieldsMap fields;
						std::string field_to_parse;
						std::stringstream stream(m[4]);
						while (std::getline(stream, field_to_parse, '\t'))
						{
							size_t colon_pos = field_to_parse.find(':');
							if (colon_pos == std::string::npos)
							{
								s_logger.Warning() << "Could not parse ctags tag field: '" << field_to_parse << "'";
								continue;
							}
							fields.insert(std::make_pair(field_to_parse.substr(0, colon_pos), field_to_parse.substr(colon_pos + 1)));
						}

						std::string scope;
						if (fields.find("class") != fields.end())
							scope = fields["class"];
						else if (fields.find("struct") != fields.end())
							scope = fields["struct"];
						else if (fields.find("namespace") != fields.end())
							scope = fields["namespace"];

						IndexEntryKind kind;
						std::string kind_str = fields.at("kind");
						if (kind_str == "class" || kind_str == "struct")
							kind = IndexEntryKind::Type;
						if (kind_str == "member")
							kind = IndexEntryKind::Variable;
						if (kind_str == "function")
							kind = IndexEntryKind::Function;

						entries.push_back(std::make_shared<CTagsIndexEntry>(name, scope, kind, Location(_filename, line, 1)));
					}
					else
						s_logger.Warning() << "Could not parse ctags output line: '" << s << "'";
				},
				[&]() { stdout_closed.set_value(); }
			));

		stdout_closed.get_future().wait();
		ctags.reset();

		return std::make_shared<CTagsPartialIndex>(entries);
	}


	IPartialIndexPtr GenericCTagsIndexer::LoadIndex(const std::string& filename)
	{
		using namespace boost::property_tree;

		ptree root;
		std::ifstream f(filename);
		read_json(f, root);

		PTreeReader wr(root);
		std::vector<CTagsIndexEntry> ctags_entries;
		PTreeReader(root).Read("entries", ctags_entries);

		std::vector<IIndexEntryPtr> entries;
		for (const auto& e : ctags_entries)
			entries.push_back(std::make_shared<CTagsIndexEntry>(e.GetName(), e.GetScope(), e.GetKind(), Location(_filename, e.GetLocation().GetLine(), 1)));

		return std::make_shared<CTagsPartialIndex>(entries);
	}

}
