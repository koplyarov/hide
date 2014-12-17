#include <hide/buildsystems/cmake/NinjaCMakeBackend.h>

#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include <hide/utils/Executable.h>
#include <hide/utils/FileSystemUtils.h>
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

		virtual StringArray GetTargets()
		{
			return { "LOL", "WUT" };
		}

		boost::filesystem::path GetCMakeSubdirectory(const boost::filesystem::path& filepath) const
		{
			using namespace boost::filesystem;

			for (path p = filepath; !equivalent(p, current_path()); p = p.parent_path())
				if (is_regular_file(p / "CMakeLists.txt"))
					return p;

			return ".";
		}

		std::string GetProjectName(const boost::filesystem::path& subdirectory) const
		{
			using namespace boost;
			using namespace boost::filesystem;

			path cmake_lists = subdirectory / "CMakeLists.txt";

			regex re("^\\s*project\\(\\s*([^)]+)\\s*\\)\\s*$");

			std::ifstream f(cmake_lists.string());
			std::string line;
			while (std::getline(f, line))
			{
				smatch m;
				if (regex_match(line, m, re))
					return m[1];
			}

			BOOST_THROW_EXCEPTION(std::runtime_error(StringBuilder() % "Cannot find project name in " % cmake_lists));
		}

		virtual void BuildFile(const IFilePtr& file)
		{
			using namespace boost::filesystem;

			// TODO: use ninja -t query ??

			path dir = RelativePath(GetCMakeSubdirectory(file->GetFilename()), ".");
			std::string project_name = GetProjectName(dir);
			std::string filename = RelativePath(file->GetFilename(), ".").string();

			s_logger.Debug() << "CMake subdirectory: " << dir << ", project: " << project_name;

			StringArray params;
			params.push_back("-j" + std::to_string(_config->GetNumThreads()));
			if (!_config->GetBuildDir().empty())
				params.push_back("-C" + _config->GetBuildDir());
			params.push_back(StringBuilder() % (dir / "CMakeFiles" / (project_name + ".dir") / (filename + ".o")).string());

			ExecutablePtr ninja = Executable::ExecuteSync("ninja", params);
			if (ninja->Succeeded())
				s_logger.Debug() << "Build succeeded";
			else
				s_logger.Debug() << "Build failed";
		}

		virtual void BuildTarget(const std::string& target)
		{
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
