#include <hide/Project.h>

#include <boost/range/algorithm.hpp>

#include <hide/buildsystems/cmake/CMakeBuildSystem.h>
#include <hide/lang_plugins/cpp/LanguagePlugin.h>


namespace hide
{

	class Project::FileSystemNotifierListener : public virtual IFileSysterNotifierListener
	{
	private:
		Project*		_inst;

	public:
		FileSystemNotifierListener(Project* inst)
			: _inst(inst)
		{ }

		virtual void OnEvent(FileSystemNotifierTarget target, FileSystemNotifierEvent event, const std::string& path)
		{ _inst->OnFileSystemEvent(target, event, path); }
	};


	HIDE_NAMED_LOGGER(Project);

	Project::Project()
		:	_buildSystemProbers{ std::make_shared<CMakeBuildSystemProber>() },
			_langPlugins{ std::make_shared<cpp::LanguagePlugin>() },
			_files(new ProjectFiles),
			_indexer(new Indexer(_files))
	{
		_fsNotifier.reset(new FileSystemNotifier());
		_fsNotifierListener.reset(new FileSystemNotifierListener(this));
		_fsNotifier->AddListener(_fsNotifierListener);
		s_logger.Info() << "Created";
	}


	Project::~Project()
	{
		s_logger.Info() << "Destroying";
		_fsNotifier->RemoveListener(_fsNotifierListener);
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


	IndexerPtr Project::GetIndexer()
	{
		return _indexer;
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

		for (auto f : GetFiles())
			if (equivalent(f->GetFilename(), filepath))
				return f;
		return nullptr;
	}


	ProjectPtr Project::CreateAuto(const StringArray& skipRegexesList)
	{
		using namespace boost;

		std::vector<regex> skip_regexes;
		skip_regexes.reserve(skipRegexesList.size() + 1);
		skip_regexes.push_back(regex("^(.*/)?\\.hide\\b(/.*)?$"));
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

		if (is_directory(p))
		{
			_fsNotifier->AddPath(p.string());
			directory_iterator end;
			for (directory_iterator it(p); it != end; ++it)
				ScanProjectFunc(*it, skipList, indent + "  ");
		}
		else
		{
			for (auto lp : _langPlugins)
			{
				IFilePtr f = lp->ProbeFile(p.string());

				if (f)
				{
					_files->AddFile(f);
					break;
				}
			}
		}
	}


	void Project::OnFileSystemEvent(FileSystemNotifierTarget target, FileSystemNotifierEvent event, const std::string& path)
	{
		s_logger.Info() << "OnFileSystemEvent(" << target << ", " << event << ", " << path << ")";
	}

}
