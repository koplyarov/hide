#ifndef HIDE_BUILDSYSTEMS_DEFAULTBUILDPROCESS_H
#define HIDE_BUILDSYSTEMS_DEFAULTBUILDPROCESS_H


#include <hide/IBuildSystem.h>
#include <hide/buildsystems/BuildProcessBase.h>
#include <hide/utils/Executable.h>
#include <hide/utils/NamedLogger.h>


namespace hide
{

	class DefaultBuildProcess : public BuildProcessBase
	{
		class StdoutListener;

	private:
		static NamedLogger		s_logger;
		ExecutablePtr			_executable;
		IReadBufferListenerPtr	_stdoutListener;

	public:
		DefaultBuildProcess(const std::string& executable, const StringArray& parameters);
		~DefaultBuildProcess();

	private:
		void OnFinished();
	};

}

#endif
