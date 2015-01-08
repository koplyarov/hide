#ifndef HIDE_UTILS_FILESYSTEMNOTIFIER_H
#define HIDE_UTILS_FILESYSTEMNOTIFIER_H


#include <mutex>

#include <hide/utils/NamedLogger.h>
#include <hide/utils/Utils.h>


namespace hide
{

	struct FileSystemNotifierEvent
	{
		HIDE_ENUM_VALUES(Added, Removed, Modified);
		HIDE_ENUM_CLASS(FileSystemNotifierEvent);

		std::string ToString() const
		{
			switch (GetRaw())
			{
			case FileSystemNotifierEvent::Added:	return "Added";
			case FileSystemNotifierEvent::Removed:	return "Removed";
			case FileSystemNotifierEvent::Modified:	return "Modified";
			default:								BOOST_THROW_EXCEPTION(std::runtime_error(StringBuilder() % "Unknown FileSystemNotifierEvent value: " % _val));
			}
		}
	};


	struct FileSystemNotifierTarget
	{
		HIDE_ENUM_VALUES(File, Directory);
		HIDE_ENUM_CLASS(FileSystemNotifierTarget);

		std::string ToString() const
		{
			switch (GetRaw())
			{
			case FileSystemNotifierTarget::File:		return "File";
			case FileSystemNotifierTarget::Directory:	return "Directory";
			default:									BOOST_THROW_EXCEPTION(std::runtime_error(StringBuilder() % "Unknown FileSystemNotifierTarget value: " % _val));
			}
		}
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
		std::thread				_thread;

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
