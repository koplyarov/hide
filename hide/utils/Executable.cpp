#include <hide/utils/Executable.h>

#if HIDE_PLATFORM_POSIX
#	include <fcntl.h>
#	include <sys/wait.h>
#	include <unistd.h>
#endif

#include <sstream>
#include <system_error>
#include <thread>

#include <boost/scope_exit.hpp>

#include <hide/utils/PipeLinesReader.h>
#include <hide/utils/Thread.h>


namespace hide
{

#if HIDE_PLATFORM_POSIX
	class Executable::PipeWriteEnd : public virtual IPipeWriteEnd
	{
	private:
		static NamedLogger	s_logger;
		int					_fd;

	public:
		PipeWriteEnd(int fd)
			: _fd(fd)
		{ }

		~PipeWriteEnd()
		{
			Close();
		}

		virtual void Write(const ByteArray& data)
		{
			const char* ptr = data.data();
			int count = data.size();

			while (count != 0)
			{
				int ret = ::write(_fd, ptr, count);
				if (ret < 0)
					BOOST_THROW_EXCEPTION(std::system_error(errno, std::system_category(), "write failed!"));
				else if (ret > 0)
				{
					ptr += ret;
					count -= ret;
				}
			}
		}

		virtual void Close()
		{
			if (_fd != -1)
				::close(_fd);
		}
	};
	HIDE_NAMED_LOGGER(Executable::PipeWriteEnd);


	class Executable::PipeReadEnd
	{
		HIDE_NONCOPYABLE(PipeReadEnd);

	private:
		static NamedLogger		s_logger;
		int						_fd;
		IPipeReadEndHandlerPtr	_handler;
		std::thread				_thread;

	public:
		PipeReadEnd(int fd, const IPipeReadEndHandlerPtr& handler)
			: _fd(fd), _handler(handler)
		{ _thread = MakeThread("pipeReadBuffer(" + std::to_string(fd) + ")", std::bind(&PipeReadEnd::ThreadFunc, this)); }

		~PipeReadEnd()
		{
			_thread.join();
			close(_fd);
		}

	private:
		void ThreadFunc()
		{
			std::array<char, 256> local_buf;
			int ret = 0;

			BOOST_SCOPE_EXIT_ALL(&) {
				try
				{ _handler->OnEndOfData(); }
				catch (const std::exception& ex)
				{ s_logger.Error() << "An exception in pipe handler: " << ex; }
			};

			do
			{
				ret = read(_fd, local_buf.data(), local_buf.size());
				if (ret < 0)
				{
					if (errno == EBADF)
						break;
					else
						BOOST_THROW_EXCEPTION(std::system_error(errno, std::system_category(), "read failed!"));
				}
				else if (ret > 0)
				{
					try
					{ _handler->OnData(ByteArray(local_buf.begin(), local_buf.begin() + ret)); }
					catch (const std::exception& ex)
					{ s_logger.Error() << "An exception in pipe handler: " << ex; }
				}
			} while (ret > 0);
		}
	};
	HIDE_NAMED_LOGGER(Executable::PipeReadEnd);

#elif HIDE_PLATFORM_WINDOWS

#endif


	HIDE_NAMED_LOGGER(Executable);

	Executable::Executable(const std::string& executable, const StringArray& parameters, const IPipeReadEndHandlerPtr& stdoutHandler, const NamedLogger& stderrLogger)
		:	Executable(
				executable, parameters, stdoutHandler,
				std::make_shared<PipeLinesReader>([executable, &stderrLogger](const std::string& s){ stderrLogger.Warning() << executable << " stderr: " << s; })
			)
	{ }

	Executable::Executable(const std::string& executable, const StringArray& parameters, const IPipeReadEndHandlerPtr& stdoutHandler, const IPipeReadEndHandlerPtr& stderrHandler) // TODO: implement async invokation
	{
#if HIDE_PLATFORM_POSIX
		static const int read_index = 0, write_index = 1;
		int in_pipe[2], out_pipe[2], err_pipe[2];

		if (pipe(in_pipe) != 0)
			BOOST_THROW_EXCEPTION(std::system_error(errno, std::system_category(), "Could not open in_pipe"));

		if (pipe(out_pipe) != 0)
		{
			close(in_pipe[read_index]);
			close(in_pipe[write_index]);
			BOOST_THROW_EXCEPTION(std::system_error(errno, std::system_category(), "Could not open out_pipe"));
		}

		if (pipe(err_pipe) != 0)
		{
			close(in_pipe[read_index]);
			close(in_pipe[write_index]);
			close(out_pipe[read_index]);
			close(out_pipe[write_index]);
			BOOST_THROW_EXCEPTION(std::system_error(errno, std::system_category(), "Could not open err_pipe"));
		}

		pid_t pid = fork();
		if (pid)
		{
			// parent
			_pid = pid;

			close(in_pipe[read_index]);
			close(out_pipe[write_index]);
			close(err_pipe[write_index]);

			_stdin.reset(new PipeWriteEnd(in_pipe[write_index]));
			_stdout.reset(new PipeReadEnd(out_pipe[read_index], stdoutHandler));
			_stderr.reset(new PipeReadEnd(err_pipe[read_index], stderrHandler));

			_thread = MakeThread("executable(" + std::to_string(_pid) + ")", std::bind(&Executable::ThreadFunc, this));
		}
		else
		{
			// child
			close(in_pipe[write_index]);
			close(out_pipe[read_index]);
			close(err_pipe[read_index]);

			fcntl(in_pipe[read_index], F_SETFL, O_CLOEXEC);
			fcntl(out_pipe[write_index], F_SETFL, O_CLOEXEC);
			fcntl(err_pipe[write_index], F_SETFL, O_CLOEXEC);

			if (dup2(in_pipe[read_index], 0) < 0)
				exit(-1); // TODO: ???

			if (dup2(out_pipe[write_index], 1) < 0)
				exit(-1);

			if (dup2(err_pipe[write_index], 2) < 0)
				exit(-1);

			std::vector<char*> args;
			args.reserve(parameters.size() + 1);
			args.push_back(const_cast<char*>(executable.c_str()));
			for (const auto& p : parameters)
				args.push_back(const_cast<char*>(p.c_str()));
			args.push_back(nullptr);

			execvp(executable.c_str(), args.data());
			exit(-1); // TODO: ???
		}
#elif HIDE_PLATFORM_WINDOWS
		HIDE_THROW(std::runtime_error("Not implemented!"));
#else
#	error Executable::Executable is not implemented
#endif
		s_logger.Debug() << "Created, executable: " << executable << ", parameters: " << parameters;
	}


	Executable::~Executable()
	{
		s_logger.Debug() << "Destroying";
		_thread.join();
		s_logger.Debug() << "Destroyed";
	}


	void Executable::Interrupt()
	{
		s_logger.Debug() << "Executable::Interrupt()";
#if HIDE_PLATFORM_POSIX
		// TODO: suppress repeated Interrupt calls
		int ret = kill(_pid, SIGTERM);
		if (ret != 0)
		{
			if (errno == ESRCH)
				return;
			BOOST_THROW_EXCEPTION(std::system_error(errno, std::system_category(), "Could not kill " + std::to_string(_pid) + " child process!"));
		}
#elif HIDE_PLATFORM_WINDOWS
		HIDE_THROW(std::runtime_error("Not implemented!"));
#else
#	error Executable::Interrupt is not implemented
#endif
	}


	void Executable::PopulateState(const IExecutableListenerPtr& listener) const
	{
		if (_retCode)
			listener->OnFinished(*_retCode);
	}


	void Executable::ThreadFunc()
	{
#if HIDE_PLATFORM_POSIX
		s_logger.Debug() << "Waiting for a child (" << _pid << ")...";

		int status = 0, wpid = 0;
		if ((wpid = waitpid(_pid, &status, 0)) < 0)
			BOOST_THROW_EXCEPTION(std::system_error(errno, std::system_category(), "waitpid failed!"));

		_stdout.reset();
		_stderr.reset();

		HIDE_LOCK(GetMutex());
		if (WIFEXITED(status))
		{
			s_logger.Debug() << "The child (" << wpid << ") exited normally, status: " << status;
			_retCode = WEXITSTATUS(status);
		}
		else
		{
			s_logger.Debug() << "The child (" << wpid << ") failed to exit";
			_retCode = 255;
		}

		InvokeListeners(std::bind(&IExecutableListener::OnFinished, std::placeholders::_1, *_retCode));
#elif HIDE_PLATFORM_WINDOWS
		HIDE_THROW(std::runtime_error("Not implemented!"));
#else
#	error Executable::ThreadFunc is not implemented
#endif
	}

}
