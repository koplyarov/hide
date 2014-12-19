#ifndef HIDE_IFILE_H
#define HIDE_IFILE_H


#include <hide/utils/Utils.h>


namespace hide
{

	struct IFile
	{
		virtual ~IFile() { }

		virtual std::string GetFilename() const { HIDE_PURE_VIRTUAL_CALL(); }

		virtual std::string ToString() const { return GetFilename(); }
	};
	HIDE_DECLARE_PTR(IFile);

}

#endif
