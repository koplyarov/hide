#ifndef HIDE_UTILS_FILESYSTEMUTILS_H
#define HIDE_UTILS_FILESYSTEMUTILS_H


#include <boost/filesystem.hpp>


namespace hide
{

	boost::filesystem::path RelativePath(const boost::filesystem::path &path, const boost::filesystem::path &relative_to);
	bool PathContains(boost::filesystem::path dir, boost::filesystem::path file);

}

#endif
