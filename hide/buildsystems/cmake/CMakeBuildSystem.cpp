#include <hide/buildsystems/cmake/CMakeBuildSystem.h>

#include <boost/filesystem.hpp>

#include <hide/buildsystems/cmake/CMakeBuildConfig.h>
#include <hide/buildsystems/cmake/NinjaCMakeBackend.h>
#include <hide/utils/NamedLogger.h>


namespace hide
{

	class CMakeBuildSystem : public virtual IBuildSystem
	{
	private:
		static NamedLogger					s_logger;
		ICMakeBackendProberPtrArray			_backendProbers;
		StringToIBuildConfigPtrMap			_buildConfigs;
		ICMakeBackendPtr					_currentBackend; // TODO: refactor this somehow?

	public:
		CMakeBuildSystem()
			:	_backendProbers{ std::make_shared<NinjaCMakeBackendProber>() },
				_buildConfigs{ {"default", std::make_shared<CMakeBuildConfig>(1, "")} }
		{
			s_logger.Info() << "Created";
		}

		~CMakeBuildSystem()
		{
			s_logger.Info() << "Destroying";
		}

		virtual StringArray GetTargets()
		{
			s_logger.Info() << "GetTargets()";
			StringArray result = GetCurrentBackend()->GetTargets();
			s_logger.Info() << "Targets: " << result;
			return result;
		}

		virtual IBuildProcessPtr BuildFile(const IFilePtr& file)
		{
			s_logger.Info() << "BuildFile(" << file << ")";
			return GetCurrentBackend()->BuildFile(file);
		}

		virtual IBuildProcessPtr BuildAll()
		{
			return BuildTarget("");
		}

		virtual IBuildProcessPtr BuildTarget(const std::string& target)
		{
			s_logger.Info() << "BuildTarget(" << target << ")";
			return GetCurrentBackend()->BuildTarget(target);
		}

		virtual StringToIBuildConfigPtrMap GetAvailableBuildConfigs()
		{
			 return _buildConfigs;
		}

		virtual void SetAvailableBuildConfigs(const StringToIBuildConfigPtrMap& configs)
		{
			_currentBackend.reset();
			_buildConfigs = configs;
		}

	private:
		ICMakeBackendPtr GetCurrentBackend()
		{
			if (!_currentBackend)
			{
				CMakeBuildConfigPtr config = GetCurrentBuildConfig();
				for (const auto& prober : _backendProbers)
				{
					_currentBackend = prober->Probe(config);
					if (_currentBackend)
					{
						s_logger.Info() << "CMake backend for " << "default" << " build config is " << _currentBackend->GetName();
						break;
					}
				}
				if (!_currentBackend)
					s_logger.Warning() << "Could not find a CMake backend for " << "default" << " build config";
			}

			return _currentBackend;
		}

		CMakeBuildConfigPtr GetCurrentBuildConfig()
		{
			return std::dynamic_pointer_cast<CMakeBuildConfig>(_buildConfigs.at("default"));
		}
	};
	HIDE_NAMED_LOGGER(CMakeBuildSystem);


	IBuildSystemPtr CMakeBuildSystemProber::Probe() const
	{
		using namespace boost::filesystem;
		return is_regular_file("CMakeLists.txt") ? std::make_shared<CMakeBuildSystem>() : nullptr;
	}

}
