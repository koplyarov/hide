#ifndef HIDE_BUILDSYSTEMS_DEFAULTBUILDPROCESS_H
#define HIDE_BUILDSYSTEMS_DEFAULTBUILDPROCESS_H


#include <hide/IBuildSystem.h>
#include <hide/utils/Executable.h>
#include <hide/utils/NamedLogger.h>


namespace hide
{

	class DefaultBuildProcess : public IBuildProcess
	{
		class StdoutListener;

	private:
		static NamedLogger		s_logger;
		BuildLogControlPtr		_buildLogControl;
		Executable				_executable;

	public:
		DefaultBuildProcess(const std::string& executable, const StringArray& parameters);
		~DefaultBuildProcess();

		virtual BuildLogPtr GetLog() { return _buildLogControl; }

	private:
		void OnFinished();
	};

}

#endif
