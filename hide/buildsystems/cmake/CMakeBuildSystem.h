#ifndef HIDE_BUILDSYSTEMS_CMAKE_BUILDSYSTEM_H
#define HIDE_BUILDSYSTEMS_CMAKE_BUILDSYSTEM_H


#include <hide/IBuildSystem.h>
#include <hide/buildsystems/cmake/ICMakeBackend.h>


namespace hide
{

	class CMakeBuildSystemProber : public virtual IBuildSystemProber
	{
	public:
		virtual IBuildSystemPtr Probe() const;
	};

}

#endif
