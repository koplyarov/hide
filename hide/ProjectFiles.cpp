#include <hide/ProjectFiles.h>


namespace hide
{

	void ProjectFiles::AddFile(const IFilePtr& file)
	{
		HIDE_LOCK(GetMutex());
		_files.push_back(file);
		InvokeListeners(std::bind(&IProjectFilesListener::OnFileAdded, std::placeholders::_1, file));
	}


	void ProjectFiles::RemoveFile(const IFilePtr& file)
	{
		HIDE_LOCK(GetMutex());
		_files.erase(std::find(_files.begin(), _files.end(), file));
		InvokeListeners(std::bind(&IProjectFilesListener::OnFileRemoved, std::placeholders::_1, file));
	}


	void ProjectFiles::PopulateState(const IProjectFilesListenerPtr& listener) const
	{
		for (auto f : _files)
			listener->OnFileAdded(f);
	}

}
