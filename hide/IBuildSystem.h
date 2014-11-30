#ifndef HIDE_IBUILDSYSTEM_H
#define HIDE_IBUILDSYSTEM_H


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


	struct IBuildSystem
	{
		virtual ~IBuildSystem() { }

		virtual StringArray GetTargets() const = 0;

		virtual void BuildFile(const IFilePtr& file) = 0;
		virtual void BuildAll() = 0;
		virtual void BuildTarget(const std::string& target) = 0;

		virtual StringToIBuildConfigPtrMap GetAvailableBuildConfigs() const = 0;
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
