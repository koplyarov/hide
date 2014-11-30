#ifndef HIDE_IFILE_H
#define HIDE_IFILE_H


#include <hide/utils/Utils.h>


namespace hide
{

	struct IFile
	{
		virtual ~IFile() { }

		virtual std::string GetFilename() const = 0;
	};
	HIDE_DECLARE_PTR(IFile);

}

#endif
