#ifndef HIDE_IBUILDSYSTEM_H
#define HIDE_IBUILDSYSTEM_H


#include <hide/BuildLog.h>
#include <hide/IFile.h>
#include <hide/utils/Utils.h>

namespace hide
{

	struct IBuildConfig
	{
		virtual ~IBuildConfig() { }
	};
	HIDE_DECLARE_PTR(IBuildConfig);
	HIDE_DECLARE_MAP(String, IBuildConfigPtr);


	struct IBuildProcess
	{
		virtual ~IBuildProcess() { }

		virtual BuildLogPtr GetLog() { HIDE_PURE_VIRTUAL_CALL(); }
	};
	HIDE_DECLARE_PTR(IBuildProcess);


	struct IBuildSystem
	{
		virtual ~IBuildSystem() { }

		virtual StringArray GetTargets() = 0;

		virtual IBuildProcessPtr BuildFile(const IFilePtr& file) = 0;
		virtual IBuildProcessPtr BuildAll() = 0;
		virtual IBuildProcessPtr BuildTarget(const std::string& target) = 0;

		virtual StringToIBuildConfigPtrMap GetAvailableBuildConfigs() = 0;
		virtual void SetAvailableBuildConfigs(const StringToIBuildConfigPtrMap& configs) = 0;
	};
	HIDE_DECLARE_PTR(IBuildSystem);


	struct IBuildSystemProber
	{
		virtual ~IBuildSystemProber() { }

		virtual IBuildSystemPtr Probe() const = 0;
	};
	HIDE_DECLARE_PTR(IBuildSystemProber);
	HIDE_DECLARE_ARRAY(IBuildSystemProberPtr);

}

#endif
