#include <hide/utils/Executable.h>

#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

#include <sstream>
#include <thread>

#include <boost/scope_exit.hpp>

#include <hide/utils/Thread.h>


namespace hide
{

	namespace
	{
#if HIDE_PLATFORM_POSIX
		class PipeReadBuffer : public ListenersHolder<IReadBufferListener, IReadBuffer>
		{
			HIDE_NONCOPYABLE(PipeReadBuffer);

		private:
			static NamedLogger	s_logger;
			int					_fd;
			ByteArray			_data;
			std::thread			_thread;

		public:
			PipeReadBuffer(int fd)
				: _fd(fd)
			{
				//fcntl(out_pipe[read_index], F_SETFL, O_NONBLOCK | O_ASYNC); // TODO: check errors
				_thread = MakeThread("pipeReadBuffer(" + std::to_string(fd) + ")", std::bind(&PipeReadBuffer::ThreadFunc, this));
			}

			~PipeReadBuffer()
			{
				::close(_fd);
				_thread.join();
			}

			virtual ByteArray Read(int64_t ofs) const
			{
				HIDE_LOCK(GetMutex());
				if (ofs > (int64_t)_data.size())
					BOOST_THROW_EXCEPTION(std::runtime_error("Invalid offset!"));
				ByteArray result;
				int64_t result_size = _data.size() - ofs;
				result.resize(result_size);
				std::copy(_data.begin() + ofs, _data.begin() + ofs + result_size, result.begin());
				return result;
			}

		protected:
			virtual void PopulateState(const IReadBufferListenerPtr& listener) const
			{ listener->OnBufferChanged(*this); }

		private:
			void ThreadFunc()
			{
				std::array<char, 256> local_buf;
				int ret = 0;

				BOOST_SCOPE_EXIT_ALL(&) {
					InvokeListeners(std::bind(&IReadBufferListener::OnEndOfData, std::placeholders::_1));
				};

				do
				{
					ret = read(_fd, local_buf.data(), local_buf.size());
					if (ret < 0)
					{
						if (errno == EBADF)
							break;
						else
							BOOST_THROW_EXCEPTION(std::runtime_error("read failed!"));
					}
					else if (ret > 0)
					{
						HIDE_LOCK(GetMutex());
						_data.resize(_data.size() + ret);
						std::copy(local_buf.begin(), local_buf.begin() + ret, _data.end() - ret);
						InvokeListeners(std::bind(&IReadBufferListener::OnBufferChanged, std::placeholders::_1, std::ref(*this)));
					}
				} while (ret > 0);
			}
		};
		HIDE_DECLARE_PTR(PipeReadBuffer);
		HIDE_NAMED_LOGGER(PipeReadBuffer);
#endif
	}


	HIDE_NAMED_LOGGER(Executable);

	Executable::Executable(const std::string& executable, const StringArray& parameters) // TODO: implement async invokation
	{
#if HIDE_PLATFORM_POSIX
		static const int read_index = 0, write_index = 1;
		int in_pipe[2], out_pipe[2], err_pipe[2];

		if (pipe(in_pipe) != 0)
			BOOST_THROW_EXCEPTION(std::runtime_error("Could not open in_pipe"));

		if (pipe(out_pipe) != 0)
		{
			close(in_pipe[read_index]);
			close(in_pipe[write_index]);
			BOOST_THROW_EXCEPTION(std::runtime_error("Could not open out_pipe"));
		}

		if (pipe(err_pipe) != 0)
		{
			close(in_pipe[read_index]);
			close(in_pipe[write_index]);
			close(out_pipe[read_index]);
			close(out_pipe[write_index]);
			BOOST_THROW_EXCEPTION(std::runtime_error("Could not open err_pipe"));
		}

		pid_t pid = fork();
		if (pid)
		{
			// parent
			_pid = pid;

			close(in_pipe[read_index]);
			close(out_pipe[write_index]);
			close(err_pipe[write_index]);

			close(in_pipe[write_index]);
			_stdout.reset(new PipeReadBuffer(out_pipe[read_index]));
			_stderr.reset(new PipeReadBuffer(err_pipe[read_index]));

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
			for (auto p : parameters)
				args.push_back(const_cast<char*>(p.c_str()));
			args.push_back(nullptr);

			execvp(executable.c_str(), args.data());
			exit(-1); // TODO: ???
		}
#else
#	error Executable::Executable is not implemented
#endif
		s_logger.Debug() << "Created, executable: " << executable << ", parameters: " << parameters;
	}


	Executable::~Executable()
	{
		s_logger.Debug() << "Destroying";
		_thread.join();
	}


	void Executable::Interrupt()
	{
		s_logger.Debug() << "Executable::Interrupt()";
#if HIDE_PLATFORM_POSIX
		// TODO: suppress repeated Interrupt calls
		int ret = kill(_pid, SIGINT);
		if (ret != 0)
		{
			if (errno == ESRCH)
				return;
			BOOST_THROW_EXCEPTION(std::runtime_error("Could not kill " + std::to_string(_pid) + " child process!"));
		}
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
			BOOST_THROW_EXCEPTION(std::runtime_error("waitpid failed"));

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
#else
#	error Executable::ThreadFunc is not implemented
#endif
	}

}
