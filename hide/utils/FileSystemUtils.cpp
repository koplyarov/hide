#include <hide/utils/FileSystemUtils.h>

#include <hide/utils/NamedLogger.h>


namespace hide
{

	static NamedLogger g_fsUtilsLogger("FileSystemUtils");


	boost::filesystem::path RelativePath(const boost::filesystem::path &path, const boost::filesystem::path &relative_to)
	{
		namespace fs = boost::filesystem;

		// create absolute paths
		fs::path p = fs::absolute(path);
		fs::path r = fs::absolute(relative_to);

		// if root paths are different, return absolute path
		if( p.root_path() != r.root_path() )
			return p;

		// initialize relative path
		fs::path result;

		// find out where the two paths diverge
		fs::path::const_iterator itr_path = p.begin();
		fs::path::const_iterator itr_relative_to = r.begin();
		while( *itr_path == *itr_relative_to && itr_path != p.end() && itr_relative_to != r.end() ) {
			++itr_path;
			++itr_relative_to;
		}

		// add "../" for each remaining token in relative_to
		if( itr_relative_to != r.end() ) {
			++itr_relative_to;
			while( itr_relative_to != r.end() ) {
				result /= "..";
				++itr_relative_to;
			}
		}

		// add remaining path
		while( itr_path != p.end() ) {
			result /= *itr_path;
			++itr_path;
		}

		return result;
	}


	bool PathContains(boost::filesystem::path dir, boost::filesystem::path file)
	{
		using namespace boost::filesystem;
		dir.normalize();
		file.normalize();

		if (dir.filename() == ".")
			dir.remove_filename();
		file.remove_filename();

		auto dir_len = std::distance(dir.begin(), dir.end());
		auto file_len = std::distance(file.begin(), file.end());
		if (dir_len > file_len)
			return false;

		return std::equal(dir.begin(), dir.end(), file.begin());
	}


	void RemoveFileAndParentDirectories(const boost::filesystem::path& filepath, const boost::filesystem::path& stopAt)
	{
		using namespace boost::filesystem;

		path normalized_filepath = filepath;
		normalized_filepath.normalize();

		boost::system::error_code err;

		g_fsUtilsLogger.Debug() << "Removing " << normalized_filepath;
		remove(normalized_filepath, err);
		if (err)
			g_fsUtilsLogger.Warning() << "Could not remove " << normalized_filepath << ": " << err;

		path p = normalized_filepath.parent_path(), stop_at = stopAt;
		p.normalize();
		stop_at.normalize();
		for (; p != stopAt && (!exists(p, err) || is_empty(p, err)); p = p.parent_path())
		{
			g_fsUtilsLogger.Debug() << "Removing " << p;
			remove(p, err);
			if (err)
				g_fsUtilsLogger.Warning() << "Could not remove " << p << ": " << err;
		}
	}

}
