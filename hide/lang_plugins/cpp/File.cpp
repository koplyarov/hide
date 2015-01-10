#include <hide/lang_plugins/cpp/File.h>

#include <boost/filesystem.hpp>

#include <hide/lang_plugins/GenericCTagsIndexer.h>
#include <hide/utils/FileSystemUtils.h>


namespace hide {
namespace cpp
{

	using namespace boost::filesystem;

	class FileIndexableId : public Comparable<FileIndexableId>, public virtual IIndexableId
	{
	private:
		path		_path;

	public:
		FileIndexableId(const std::string& filename)
			: _path(RelativePath(filename, current_path()))
		{ }

		virtual std::string ToString() const { return _path.string(); }

	protected:
		virtual int DoCompare(const FileIndexableId& other) const
		{
			if (_path < other._path)
				return -1;
			if (other._path < _path)
				return 1;
			return 0;
		}
	};


	File::File(const std::string& filename)
		: _filename(filename)
	{ }


	IIndexableIdPtr File::GetIndexableId()
	{ return std::make_shared<FileIndexableId>(_filename); }


	Time File::GetModificationTime()
	{ return last_write_time(_filename); }


	IPartialIndexerPtr File::GetIndexer()
	{ return std::make_shared<GenericCTagsIndexer>(_filename); }

}}
