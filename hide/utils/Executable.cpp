#include <hide/utils/Executable.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <sstream>
#include <thread>

#include <boost/scope_exit.hpp>

#include <hide/utils/ReadBufferBase.h>


namespace hide
{

	namespace
	{
#if HIDE_PLATFORM_POSIX
		class PipeReadBuffer : public ReadBufferBase
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
				_thread = std::thread(std::bind(&PipeReadBuffer::ThreadFunc, this));
			}

			~PipeReadBuffer()
			{
				::close(_fd);
				_thread.join();
			}

			virtual ByteArray Read(int64_t ofs) const
			{
				std::lock_guard<std::recursive_mutex> l(_mutex);
				if (ofs > _data.size())
					BOOST_THROW_EXCEPTION(std::runtime_error("Invalid offset!"));
				ByteArray result;
				int64_t result_size = _data.size() - ofs;
				result.resize(result_size);
				std::copy(_data.begin() + ofs, _data.begin() + ofs + result_size, result.begin());
				return result;
			}

		private:
			void ThreadFunc()
			{
				std::array<char, 256> local_buf;
				int ret = 0;
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
						std::lock_guard<std::recursive_mutex> l(_mutex);
						_data.resize(_data.size() + ret);
						std::copy(local_buf.begin(), local_buf.begin() + ret, _data.end() - ret);
						for (auto l : _listeners)
							l->OnBufferChanged(*this);
					}
				} while (ret >= 0);
			}
		};
		HIDE_DECLARE_PTR(PipeReadBuffer);
		HIDE_NAMED_LOGGER(PipeReadBuffer);
#endif
	}

	class TestReadBufferListener : public virtual IReadBufferListener
	{
	private:
		static NamedLogger	s_logger;
		int64_t				_ofs;

	public:
		TestReadBufferListener()
			: _ofs(0)
		{ }

		virtual void OnBufferChanged(const IReadBuffer& buf)
		{
			ByteArray data = buf.Read(_ofs);
			_ofs += data.size();
			data.push_back('\0');
			printf("%s", data.data());
		}
	};
	HIDE_NAMED_LOGGER(TestReadBufferListener);


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
			s_logger.Debug() << "Waiting for a child (" << pid << ")...";

			close(in_pipe[read_index]);
			close(out_pipe[write_index]);
			close(err_pipe[write_index]);

			IReadBufferPtr out_buf(new PipeReadBuffer(out_pipe[read_index]));
			out_buf->AddListener(std::make_shared<TestReadBufferListener>());
			_stdout = out_buf;
			_stderr.reset(new PipeReadBuffer(err_pipe[read_index]));

			int status = 0, wpid = 0;
			if ((wpid = waitpid(pid, &status, 0)) < 0)
				BOOST_THROW_EXCEPTION(std::runtime_error("waitpid failed"));

			if (WIFEXITED(status))
			{
				s_logger.Debug() << "The child (" << wpid << ") exited normally, status: " << status;
				_retCode = WEXITSTATUS(status) >> 8;
			}
			else
			{
				s_logger.Debug() << "The child (" << wpid << ") failed to exit";
				_retCode = 255;
			}

			//std::string out(4096, '\0');
			//fcntl(out_pipe[read_index], F_SETFL, O_NONBLOCK | O_ASYNC);
			//int ret = read(out_pipe[read_index], &out[0], out.size());
			//out.resize(ret >= 0 ? ret : 0);

			//std::string err(4096, '\0');
			//fcntl(err_pipe[read_index], F_SETFL, O_NONBLOCK | O_ASYNC);
			//ret = read(err_pipe[read_index], &err[0], err.size());
			//err.resize(ret >= 0 ? ret : 0);

			//if (!err.empty())
				//s_logger.Debug() << "Subprocess stderr: " << err;
			//if (!out.empty())
				//s_logger.Debug() << "Subprocess stdout: " << out;

			close(in_pipe[write_index]);
			//close(out_pipe[read_index]);
			//close(err_pipe[read_index]);
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
#	error Executable::ExecuteSync is not implemented
#endif
	}


	ExecutablePtr Executable::ExecuteSync(const std::string& executable, const StringArray& parameters)
	{
		ExecutablePtr result(new Executable(executable, parameters));
		// TODO: wait for the execution to complete
		return result;
	}

}
