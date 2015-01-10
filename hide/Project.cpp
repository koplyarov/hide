#include <hide/Project.h>

#include <boost/range/algorithm.hpp>

#include <hide/buildsystems/cmake/CMakeBuildSystem.h>
#include <hide/lang_plugins/cpp/LanguagePlugin.h>
#include <hide/utils/FileSystemUtils.h>


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

	Project::Project(const RegexesVector& skipList)
		:	_skipList(skipList),
			_buildSystemProbers{ std::make_shared<CMakeBuildSystemProber>() },
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
		{
			path p(filepath);
			p.normalize();

			path fp(f->GetFilename());
			fp.normalize();

			if (p == fp)
				return f;
		}
		return nullptr;
	}


	ProjectPtr Project::CreateAuto(const StringArray& skipRegexesList)
	{
		using namespace boost;

		std::vector<regex> skip_regexes;
		skip_regexes.reserve(skipRegexesList.size() + 1);
		skip_regexes.push_back(regex("^(.*/)?\\.hide\\b(/.*)?$"));
		transform(skipRegexesList, std::back_inserter(skip_regexes), [](const std::string& s) { return regex(s); });

		ProjectPtr result(new Project(skip_regexes));
		result->ScanProjectFunc(".");
		return result;
	}


	void Project::ScanProjectFunc(const boost::filesystem::path& p)
	{
		using namespace boost;
		using namespace boost::filesystem;

		if (find_if(_skipList, [&p](const regex& re) { smatch m; return regex_match(p.string(), m, re); }) != _skipList.end())
			return;

		if (is_directory(p))
		{
			_fsNotifier->AddPath(p.string());
			directory_iterator end;
			for (directory_iterator it(p); it != end; ++it)
				ScanProjectFunc(*it);
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
		switch (event.GetRaw())
		{
		case FileSystemNotifierEvent::Added:
			ScanProjectFunc(path);
			break;
		case FileSystemNotifierEvent::Removed:
			switch(target.GetRaw())
			{
			case FileSystemNotifierTarget::File:
				{
					IFilePtr file = GetFileByPath(path);
					if (file)
						_files->RemoveFile(file); // TODO: reimplement
				}
				break;
			case FileSystemNotifierTarget::Directory:
				for (auto f : _files->GetFiles())
				{
					if (PathContains(path, f->GetFilename()))
						_files->RemoveFile(f); // TODO: reimplement
				}
				break;
			}
			break;
		case FileSystemNotifierEvent::Modified:
			switch(target.GetRaw())
			{
			case FileSystemNotifierTarget::File:
				{
					IFilePtr file = GetFileByPath(path);
					if (file)
						_files->ReportModified(file); // TODO: reimplement
				}
				break;
			case FileSystemNotifierTarget::Directory:
				s_logger.Warning() << __func__ << "target: " << target << ", event: " << event <<  " - Not implemented!";
				break;
			}
			break;
		}
	}

}
