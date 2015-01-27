#ifndef HIDE_LANG_PLUGINS_CPP_ICPPCOMPILATIONINFO_H
#define HIDE_LANG_PLUGINS_CPP_ICPPCOMPILATIONINFO_H


#include <boost/filesystem/path.hpp>

#include <hide/utils/Utils.h>


namespace hide
{

	struct ICppCompilationInfo
	{
		virtual ~ICppCompilationInfo() { }

		virtual StringArray GetOptions(const boost::filesystem::path& file) = 0;
	};
	HIDE_DECLARE_PTR(ICppCompilationInfo);

}

#endif
