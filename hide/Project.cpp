#include <hide/Project.h>

#include <boost/range/algorithm.hpp>

#include <hide/buildsystems/cmake/CMakeBuildSystem.h>
#include <hide/lang_plugins/cpp/LanguagePlugin.h>


namespace hide
{

	HIDE_NAMED_LOGGER(Project);

	Project::Project()
		:	_buildSystemProbers{ std::make_shared<CMakeBuildSystemProber>() },
			_langPlugins{ std::make_shared<cpp::LanguagePlugin>() }
	{
		s_logger.Debug() << "Created";
	}


	Project::~Project()
	{
		s_logger.Debug() << "Destroying";
	}


	IBuildSystemPtr Project::GetBuildSystem()
	{
		if (!_currentBuildSystem)
			for (auto prober : _buildSystemProbers)
			{
				_currentBuildSystem = prober->Probe();
				if (_currentBuildSystem)
					break;
			}

		return _currentBuildSystem;
	}


    void Project::AddBuffer(const BufferPtr& buffer)
    {
		std::string name = buffer->GetName();
        HIDE_CHECK(_buffers.find(name) == _buffers.end(), std::runtime_error("Buffer " + name + " already registered!"));
        _buffers.insert(std::make_pair(name, buffer));
    }


    void Project::RemoveBuffer(const std::string& bufferName)
    {
        _buffers.erase(bufferName);
    }


	IFilePtr Project::GetFileByPath(const std::string& filepath)
	{
		using namespace boost::filesystem;

		for (auto f : _files)
			if (equivalent(f->GetFilename(), filepath))
				return f;
		return nullptr;
	}


	ProjectPtr Project::CreateAuto(const StringArray& skipRegexesList)
	{
		using namespace boost;

		std::vector<regex> skip_regexes;
		skip_regexes.reserve(skipRegexesList.size());
		transform(skipRegexesList, std::back_inserter(skip_regexes), [](const std::string& s) { return regex(s); });

		ProjectPtr result(new Project);
		result->ScanProjectFunc(".", skip_regexes);
		return result;
	}


	void Project::ScanProjectFunc(const boost::filesystem::path& p, const std::vector<boost::regex>& skipList, const std::string& indent)
	{
		using namespace boost;
		using namespace boost::filesystem;

		if (find_if(skipList, [&p](const regex& re) { smatch m; return regex_match(p.string(), m, re); }) != skipList.end())
			return;

		for (std::vector<ILanguagePluginPtr>::const_iterator it = _langPlugins.begin(); it != _langPlugins.end(); ++it)
		{
			IFilePtr f = (*it)->ProbeFile(p.string());

			if (f)
			{
				_files.push_back(f);
				break;
			}
		}

		if (is_directory(p))
		{
			directory_iterator end;
			for (directory_iterator it(p); it != end; ++it)
				ScanProjectFunc(*it, skipList, indent + "  ");
		}
	}

}
