#ifndef HIDE_IBUILDSYSTEM_H
#define HIDE_IBUILDSYSTEM_H


#include <hide/BuildLogLine.h>
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


	struct BuildStatus
	{
		HIDE_ENUM_VALUES(Succeeded, Failed, Interrupted);
		HIDE_ENUM_CLASS(BuildStatus);

		HIDE_DECLARE_SWIG_TO_STRING_WRAPPER();
	};


	struct IBuildProcessListener
	{
		virtual ~IBuildProcessListener() { }

		virtual void OnLine(const BuildLogLine& line) { }
		virtual void OnFinished(BuildStatus status) { }
	};
	HIDE_DECLARE_PTR(IBuildProcessListener);


	struct IBuildProcess
	{
		virtual ~IBuildProcess() { }

		virtual void Interrupt() { HIDE_PURE_VIRTUAL_CALL(); }

		virtual void AddListener(const IBuildProcessListenerPtr& listener) { HIDE_PURE_VIRTUAL_CALL(); }
		virtual void RemoveListener(const IBuildProcessListenerPtr& listener) { HIDE_PURE_VIRTUAL_CALL(); }
	};
	HIDE_DECLARE_PTR(IBuildProcess);


	struct IBuildSystem
	{
		virtual ~IBuildSystem() { }

		virtual StringArray GetTargets() { HIDE_PURE_VIRTUAL_CALL(); }

		virtual IBuildProcessPtr BuildFile(const IFilePtr& file) { HIDE_PURE_VIRTUAL_CALL(); }
		virtual IBuildProcessPtr BuildAll() { HIDE_PURE_VIRTUAL_CALL(); }
		virtual IBuildProcessPtr BuildTarget(const std::string& target) { HIDE_PURE_VIRTUAL_CALL(); }

		virtual StringToIBuildConfigPtrMap GetAvailableBuildConfigs() { HIDE_PURE_VIRTUAL_CALL(); }
		virtual void SetAvailableBuildConfigs(const StringToIBuildConfigPtrMap& configs) { HIDE_PURE_VIRTUAL_CALL(); }
	};
	HIDE_DECLARE_PTR(IBuildSystem);


	struct IBuildSystemProber
	{
		virtual ~IBuildSystemProber() { }

		virtual IBuildSystemPtr Probe() const { HIDE_PURE_VIRTUAL_CALL(); }
	};
	HIDE_DECLARE_PTR(IBuildSystemProber);
	HIDE_DECLARE_ARRAY(IBuildSystemProberPtr);

}

#endif
