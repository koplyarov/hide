#ifndef HIDE_UTILS_FILESYSTEMNOTIFIER_H
#define HIDE_UTILS_FILESYSTEMNOTIFIER_H


#include <hide/utils/NamedLogger.h>
#include <hide/utils/Utils.h>
#include <hide/utils/rethread.h>

#include <mutex>


namespace hide
{

	struct FileSystemNotifierEvent
	{
		HIDE_ENUM_VALUES(Added, Removed, Modified);
		HIDE_ENUM_CLASS(FileSystemNotifierEvent);
	};


	struct FileSystemNotifierTarget
	{
		HIDE_ENUM_VALUES(File, Directory);
		HIDE_ENUM_CLASS(FileSystemNotifierTarget);
	};


	struct IFileSysterNotifierListener
	{
		virtual ~IFileSysterNotifierListener() { }

		virtual void OnEvent(FileSystemNotifierTarget target, FileSystemNotifierEvent event, const std::string& path) { HIDE_PURE_VIRTUAL_CALL(); }
	};
	HIDE_DECLARE_PTR(IFileSysterNotifierListener);


	class FileSystemNotifier
	{
		HIDE_NONCOPYABLE(FileSystemNotifier);

		class Impl;
		HIDE_DECLARE_PTR(Impl);

	private:
		static NamedLogger		s_logger;
		ImplPtr					_impl;
		thread					_thread;

	public:
		FileSystemNotifier();
		~FileSystemNotifier();

		void AddListener(const IFileSysterNotifierListenerPtr& listener);
		void RemoveListener(const IFileSysterNotifierListenerPtr& listener);

		void AddPath(const std::string& p);
		void RemovePath(const std::string& p);

	private:
		void ThreadFunc();
	};
	HIDE_DECLARE_PTR(FileSystemNotifier);

}

#endif
