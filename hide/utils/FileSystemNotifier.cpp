#include <hide/utils/FileSystemNotifier.h>

#include <system_error>

#include <boost/scope_exit.hpp>

#ifdef HIDE_PLATFORM_POSIX
#	include <limits.h>
#	include <signal.h>
#	include <sys/inotify.h>
#endif

#include <hide/utils/ListenersHolder.h>


namespace hide
{

#ifdef HIDE_PLATFORM_POSIX
	class FileSystemNotifier::Impl : public ListenersHolder<IFileSysterNotifierListener>
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
			: _interrupted(false)
		{
			_fd = inotify_init();
			HIDE_CHECK(_fd >= 0, std::runtime_error("inotify_init failed!"));

			HIDE_CHECK(pipe(_interruptPipe) == 0, std::system_error(errno, std::system_category(), "pipe failed!"));
		}

		~Impl()
		{
			if (close(_fd) != 0)
				FileSystemNotifier::s_logger.Error() << "Could not close inotify fd!";
		}

		void Interrupt()
		{
			{
				HIDE_LOCK(_mutex);
				_interrupted = true;
			}
			char c = 0;
			if (write(_interruptPipe[1], &c, 1) != 1)
				FileSystemNotifier::s_logger.Error() << "Could not write to _interruptPipe!";
		}

		void AddPath(const std::string& p)
		{
			HIDE_LOCK(_mutex);
			HIDE_CHECK(_pathToWd.find(p) == _pathToWd.end(), std::runtime_error("The path '" + p + "' has already been added!"));
			int wd = inotify_add_watch(_fd, p.c_str(), IN_MODIFY | IN_CREATE | IN_DELETE | IN_DELETE_SELF | IN_MOVED_FROM | IN_MOVED_TO);
			HIDE_CHECK(wd >= 0, std::system_error(errno, std::system_category(), "inotify_add_watch failed!"));
			_pathToWd.insert(std::make_pair(p, wd));
			_wdToPath.insert(std::make_pair(wd, p));
		}

		void RemovePath(const std::string& p)
		{
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

			int ofs = 0;
			while (ofs < ret)
			{
				auto event = reinterpret_cast<const struct inotify_event*>(buf.data() + ofs);

				auto it = _wdToPath.find(event->wd);

				std::string name = std::string(event->name, event->len);
				size_t term_null = name.find_first_of('\0');
				name = name.substr(0, term_null);

				FileSystemNotifier::s_logger.Debug() << "wd: " << it->second <<  ", name: " << name << ", mask: " << Hex(event->mask);

				std::string path = name.empty() ? it->second : (it->second + "/" + name);

				FileSystemNotifierEvent event_type = FileSystemNotifierEvent::Modified;
				if ((event->mask & IN_MODIFY) != 0)
					event_type = FileSystemNotifierEvent::Modified;
				else if ((event->mask & IN_CREATE) != 0 || (event->mask & IN_MOVED_TO) != 0)
					event_type = FileSystemNotifierEvent::Added;
				else if ((event->mask & IN_DELETE) != 0 || (event->mask & IN_MOVED_FROM) != 0 || (event->mask & IN_DELETE_SELF) != 0)
					event_type = FileSystemNotifierEvent::Removed;
				else if ((event->mask & IN_IGNORED) != 0)
					break;
				else
				{
					FileSystemNotifier::s_logger.Warning() << "Unknown event->mask: " << Hex(event->mask);
					break;
				}

				FileSystemNotifierTarget target = (event->mask & IN_ISDIR) != 0 ? FileSystemNotifierTarget::Directory : FileSystemNotifierTarget::File;

				InvokeListeners(std::bind(&IFileSysterNotifierListener::OnEvent, std::placeholders::_1, target, event_type, path));

				ofs += sizeof(struct inotify_event) + event->len;
			}

			return true;
		}

	protected:
		void PopulateState(const IFileSysterNotifierListenerPtr& listener) const
		{ }
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


	void FileSystemNotifier::AddListener(const IFileSysterNotifierListenerPtr& listener)
	{ _impl->AddListener(listener); }


	void FileSystemNotifier::RemoveListener(const IFileSysterNotifierListenerPtr& listener)
	{ _impl->RemoveListener(listener); }


	void FileSystemNotifier::AddPath(const std::string& p)
	{
		s_logger.Debug() << "AddPath(" << p << ")";
		_impl->AddPath(p);
	}


	void FileSystemNotifier::RemovePath(const std::string& p)
	{
		s_logger.Debug() << "RemovePath(" << p << ")";
		_impl->RemovePath(p);
	}


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
