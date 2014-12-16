#include <hide/utils/Executable.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <sstream>


namespace hide
{

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

			std::string out(4096, '\0');
			fcntl(out_pipe[read_index], F_SETFL, O_NONBLOCK | O_ASYNC);
			int ret = read(out_pipe[read_index], &out[0], out.size());
			out.resize(ret >= 0 ? ret : 0);

			std::string err(4096, '\0');
			fcntl(err_pipe[read_index], F_SETFL, O_NONBLOCK | O_ASYNC);
			ret = read(err_pipe[read_index], &err[0], err.size());
			err.resize(ret >= 0 ? ret : 0);

			if (!err.empty())
				s_logger.Debug() << "Subprocess stderr: " << err;
			if (!out.empty())
				s_logger.Debug() << "Subprocess stdout: " << out;

			close(in_pipe[write_index]);
			close(out_pipe[read_index]);
			close(err_pipe[read_index]);
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
