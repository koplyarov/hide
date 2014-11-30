#ifndef HIDE_BUILDSYSTEMS_CMAKE_IBACKEND_H
#define HIDE_BUILDSYSTEMS_CMAKE_IBACKEND_H


#include <hide/IBuildSystem.h>
#include <hide/IFile.h>
#include <hide/utils/Utils.h>
#include <hide/buildsystems/cmake/CMakeBuildConfig.h>

namespace hide
{

	struct ICMakeBackend
	{
		virtual ~ICMakeBackend() { }

		virtual std::string GetName() const = 0;

		virtual StringArray GetTargets() const = 0;
		virtual void BuildFile(const IFilePtr& file) = 0;
		virtual void BuildTarget(const std::string& target) = 0;
	};
	HIDE_DECLARE_PTR(ICMakeBackend);


	struct ICMakeBackendProber
	{
		virtual ~ICMakeBackendProber() { }

		virtual ICMakeBackendPtr Probe(const CMakeBuildConfigPtr& config) const = 0;
	};
	HIDE_DECLARE_PTR(ICMakeBackendProber);
	HIDE_DECLARE_ARRAY(ICMakeBackendProberPtr);

}

#endif
