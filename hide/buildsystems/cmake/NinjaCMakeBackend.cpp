#include <hide/buildsystems/cmake/NinjaCMakeBackend.h>

#include <boost/filesystem.hpp>

#include <hide/utils/Executable.h>
#include <hide/utils/NamedLogger.h>


namespace hide
{

	class NinjaCMakeBackend : public virtual ICMakeBackend
	{
	private:
		static NamedLogger		s_logger;
		CMakeBuildConfigPtr		_config;

	public:
		NinjaCMakeBackend(const CMakeBuildConfigPtr& config)
			: _config(config)
		{ }

		virtual std::string GetName() const
		{ return "Ninja"; }

		virtual StringArray GetTargets() const
		{
			return { };
		}

		virtual void BuildFile(const IFilePtr& file)
		{
		}

		virtual void BuildTarget(const std::string& target)
		{
			s_logger.Debug() << "BuildTarget(" << target << ")";

			StringArray params;
			params.push_back("-j" + std::to_string(_config->GetNumThreads()));
			if (!_config->GetBuildDir().empty())
				params.push_back("-C" + _config->GetBuildDir());

			ExecutablePtr ninja = Executable::ExecuteSync("ninja", params);
			if (ninja->Succeeded())
				s_logger.Debug() << "Build succeeded";
			else
				s_logger.Debug() << "Build failed";
		}
	};

	HIDE_NAMED_LOGGER(NinjaCMakeBackend);


	ICMakeBackendPtr NinjaCMakeBackendProber::Probe(const CMakeBuildConfigPtr& config) const
	{
		using namespace boost::filesystem;

		return is_regular_file(path(config->GetBuildDir()) / "build.ninja") ? std::make_shared<NinjaCMakeBackend>(config) : nullptr;
	}

}
