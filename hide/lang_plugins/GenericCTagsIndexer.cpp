#include <hide/lang_plugins/GenericCTagsIndexer.h>

#include <fstream>
#include <future>
#include <sstream>

#include <boost/algorithm/string.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/regex.hpp>

#include <hide/utils/Executable.h>
#include <hide/utils/MembersVisitor.h>
#include <hide/utils/PTree.h>
#include <hide/utils/ReadBufferLinesListener.h>


namespace hide
{


	class CTagsIndexEntry : public virtual IIndexEntry
	{
	private:
		std::string		_name;
		std::string		_fullName;
		Location		_location;

	public:
		CTagsIndexEntry()
		{ }

		CTagsIndexEntry(const std::string& name, const std::string& fullName, const Location& location)
			: _name(name), _fullName(fullName), _location(location)
		{ }

		virtual std::string GetName() const		{ return _name; }
		virtual std::string GetFullName() const	{ return _fullName; }
		virtual Location GetLocation() const	{ return _location; }

		HIDE_DECLARE_MEMBERS("name", &CTagsIndexEntry::_name, "fullName", &CTagsIndexEntry::_fullName, "location", &CTagsIndexEntry::_location)
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

			PTreeWriter wr;
			std::vector<CTagsIndexEntry> ctags_entries;
			std::transform(_entries.begin(), _entries.end(), std::back_inserter(ctags_entries), [](const IIndexEntryPtr& e) { return dynamic_cast<const CTagsIndexEntry&>(*e); });
			wr.Write("entries", ctags_entries);

			std::ofstream f(filename, std::ios_base::trunc);
			write_json(f, wr.GetNode());
		}

		virtual Time GetModificationTime() { BOOST_THROW_EXCEPTION(std::runtime_error("Not implemented")); }
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

		s_logger.Info() << "BuildIndex(), filename: " << _filename;

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

						std::string fullname;
						if (fields.find("class") != fields.end())
							fullname = fields["class"] + "::" + name;
						else if (fields.find("struct") != fields.end())
							fullname = fields["struct"] + "::" + name;
						else if (fields.find("namespace") != fields.end())
							fullname = fields["namespace"] + "::" + name;
						else
							fullname = name;

						entries.push_back(std::make_shared<CTagsIndexEntry>(name, fullname, Location(_filename, line, 1)));
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

		s_logger.Info() << "Load(" << filename << ")";

		ptree root;
		std::ifstream f(filename);
		read_json(f, root);

		PTreeReader wr(root);
		std::vector<CTagsIndexEntry> ctags_entries;
		PTreeReader(root).Read("entries", ctags_entries);

		std::vector<IIndexEntryPtr> entries;
		std::transform(ctags_entries.begin(), ctags_entries.end(), std::back_inserter(entries), [](const CTagsIndexEntry& e) { return std::make_shared<CTagsIndexEntry>(e); });

		return std::make_shared<CTagsPartialIndex>(entries);
	}

}
