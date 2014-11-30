#ifndef HIDE_BUILDSYSTEMS_CMAKE_BUILDCONFIG_H
#define HIDE_BUILDSYSTEMS_CMAKE_BUILDCONFIG_H


#include <string>

#include <hide/IBuildSystem.h>


namespace hide
{

	class CMakeBuildConfig : public virtual IBuildConfig
	{
	private:
		size_t			_numThreads;
		std::string		_buildDir;

	public:
		CMakeBuildConfig(size_t numThreads, const std::string buildDir)
			: _numThreads(numThreads), _buildDir(buildDir)
		{ }

		size_t GetNumThreads() const	{ return _numThreads; }
		std::string GetBuildDir() const	{ return _buildDir; }
	};
	HIDE_DECLARE_PTR(CMakeBuildConfig);

}

#endif
