#ifndef HIDE_PROJECTFILES_H
#define HIDE_PROJECTFILES_H


#include <mutex>
#include <vector>

#include <hide/IFile.h>
#include <hide/utils/ListenersHolder.h>
#include <hide/utils/Utils.h>


namespace hide
{

	struct IProjectFilesListener
	{
		virtual ~IProjectFilesListener() { }

		virtual void OnFileAdded(const IFilePtr& file) { HIDE_PURE_VIRTUAL_CALL(); }
		virtual void OnFileRemoved(const IFilePtr& file) { HIDE_PURE_VIRTUAL_CALL(); }
	};
	HIDE_DECLARE_PTR(IProjectFilesListener);


	class ProjectFiles : public ListenersHolder<IProjectFilesListener>
	{
	public:
		typedef std::vector<IFilePtr>				FilesVector;

	private:
		FilesVector						_files;

	public:
		void AddFile(const IFilePtr& file);
		void RemoveFile(const IFilePtr& file);

		FilesVector GetFiles() const
		{
			HIDE_LOCK(GetMutex());
			return _files;
		}

	protected:
		virtual void PopulateState(const IProjectFilesListenerPtr& listener) const;
	};
	HIDE_DECLARE_PTR(ProjectFiles);

}

#endif
