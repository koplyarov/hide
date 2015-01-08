#ifndef HIDE_UTILS_FILESYSTEMNOTIFIER_H
#define HIDE_UTILS_FILESYSTEMNOTIFIER_H


#include <mutex>

#include <hide/utils/NamedLogger.h>
#include <hide/utils/Utils.h>


namespace hide
{

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

		void AddPath(const std::string& p);
		void RemovePath(const std::string& p);

	private:
		void ThreadFunc();
	};
	HIDE_DECLARE_PTR(FileSystemNotifier);

}

#endif
