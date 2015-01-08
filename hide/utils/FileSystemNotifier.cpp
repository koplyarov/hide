#include <hide/utils/FileSystemNotifier.h>

#include <system_error>

#include <boost/scope_exit.hpp>

#ifdef HIDE_PLATFORM_POSIX
#	include <limits.h>
#	include <signal.h>
#	include <sys/inotify.h>
#endif


namespace hide
{

#ifdef HIDE_PLATFORM_POSIX
	class FileSystemNotifier::Impl
	{
		HIDE_NONCOPYABLE(Impl);

		typedef std::map<std::string, int>	PathToWatchDescriptor;
		typedef std::map<int, std::string>	WatchDescriptorToPath;

	private:
		std::recursive_mutex		_mutex;
		int							_fd;
		bool						_interrupted;
		int							_interruptPipe[2];
		PathToWatchDescriptor		_pathToWd;
		WatchDescriptorToPath		_wdToPath;

	public:
		Impl()
		{
			_fd = inotify_init();
			HIDE_CHECK(_fd >= 0, std::runtime_error("inotify_init failed!"));

			HIDE_CHECK(pipe(_interruptPipe) == 0, std::system_error(errno, std::system_category(), "pipe failed!"));

			AddPath(".");
		}

		~Impl()
		{
			if (close(_fd) != 0)
				s_logger.Error() << "Could not close inotify fd!";
		}

		void Interrupt()
		{
			{
				HIDE_LOCK(_mutex);
				_interrupted = true;
			}
			char c = 0;
			if (write(_interruptPipe[1], &c, 1) != 1)
				s_logger.Error() << "Could not write to _interruptPipe!";
		}

		void AddPath(const std::string& p)
		{
			s_logger.Info() << "AddPath(" << p << ")";
			HIDE_LOCK(_mutex);
			HIDE_CHECK(_pathToWd.find(p) == _pathToWd.end(), std::runtime_error("The path '" + p + "' has already been added!"));
			int wd = inotify_add_watch(_fd, p.c_str(), IN_ATTRIB | IN_MODIFY | IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO);
			HIDE_CHECK(wd >= 0, std::system_error(errno, std::system_category(), "inotify_add_watch failed!"));
			_pathToWd.insert(std::make_pair(p, wd));
			_wdToPath.insert(std::make_pair(wd, p));
		}

		void RemovePath(const std::string& p)
		{
			s_logger.Info() << "RemovePath(" << p << ")";
			HIDE_LOCK(_mutex);
			auto ptwd_it = _pathToWd.find(p);
			HIDE_CHECK(ptwd_it != _pathToWd.end(), std::runtime_error("The path '" + p + "' has not been added!"));
			int ret = inotify_rm_watch(_fd, ptwd_it->second);
			auto wdtp_it = _wdToPath.find(ptwd_it->second);
			if (wdtp_it != _wdToPath.end())
				_wdToPath.erase(wdtp_it);
			_pathToWd.erase(ptwd_it);
			HIDE_CHECK(ret >= 0, std::system_error(errno, std::system_category(), "inotify_rm_watch failed!"));
		}

		bool ProcessEvents()
		{
			s_logger.Debug() << "ProcessEvents()";
			std::array<char, sizeof(struct inotify_event) + NAME_MAX + 1>	buf;

			fd_set rfds;
			FD_ZERO(&rfds);
			FD_SET(_fd, &rfds);
			FD_SET(_interruptPipe[0], &rfds);
			int ret = select(std::max(_fd, _interruptPipe[0]) + 1, &rfds, NULL, NULL, NULL);
			HIDE_CHECK(ret >= 0, std::system_error(errno, std::system_category(), "select failed!"));

			{
				HIDE_LOCK(_mutex);
				if (_interrupted)
					return false;
			}

			ret = read(_fd, buf.data(), buf.size());
			if (ret < 0 && errno == EINTR)
				return true;
			HIDE_CHECK(ret >= 0, std::system_error(errno, std::system_category(), "read failed!"));

			auto event = reinterpret_cast<const struct inotify_event*>(buf.data());
			s_logger.Warning() << "EVENT: { path: '" << event->name << "', mask: " << event->mask << " }";

			return true;
		}
	};
#else
#	error No FileSystemNotifier::Impl class!
#endif


	HIDE_NAMED_LOGGER(FileSystemNotifier);

	FileSystemNotifier::FileSystemNotifier()
		: _impl(new Impl)
	{
		_thread = MakeThread("fileSystemNotifier", std::bind(&FileSystemNotifier::ThreadFunc, this));
		s_logger.Debug() << "Created";
	}


	FileSystemNotifier::~FileSystemNotifier()
	{
		s_logger.Debug() << "Destroying";
		_impl->Interrupt();
		s_logger.Debug() << "Joining worker thread...";
		_thread.join();
		_impl.reset();
	}


	void FileSystemNotifier::AddPath(const std::string& p)
	{ _impl->AddPath(p); }


	void FileSystemNotifier::RemovePath(const std::string& p)
	{ _impl->RemovePath(p); }


	void FileSystemNotifier::ThreadFunc()
	{
		s_logger.Debug() << "ThreadFunc begin";
		BOOST_SCOPE_EXIT_ALL(&) {
			s_logger.Debug() << "ThreadFunc end";
		};

		while (_impl->ProcessEvents())
		{ }
	}

}
