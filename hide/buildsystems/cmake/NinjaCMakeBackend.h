#ifndef HIDE_BUILDSYSTEMS_CMAKE_NINJABACKEND_H
#define HIDE_BUILDSYSTEMS_CMAKE_NINJABACKEND_H


#include <hide/buildsystems/cmake/ICMakeBackend.h>


namespace hide
{

	class NinjaCMakeBackendProber : public virtual ICMakeBackendProber
	{
		virtual ICMakeBackendPtr Probe(const CMakeBuildConfigPtr& config) const;
	};

}

#endif
