#include <hide/fs/FsMonitor.h>

#include <boost/range/algorithm.hpp>

#include <hide/utils/OperationProfiler.h>


namespace hide {
namespace fs
{

	class FsMonitor::FileSystemNotifierListener : public virtual IFileSysterNotifierListener
	{
	private:
		FsMonitor*		_inst;

	public:
		FileSystemNotifierListener(FsMonitor* inst)
			: _inst(inst)
		{ }

		virtual void OnEvent(FileSystemNotifierTarget target, FileSystemNotifierEvent event, const std::string& path)
		{ _inst->OnFileSystemEvent(target, event, path); }
	};


	HIDE_NAMED_LOGGER(FsMonitor);

	FsMonitor::FsMonitor(boost::filesystem::path rootPath, const RegexesVector& skipList)
		: _rootPath(rootPath), _skipList(skipList)
	{
		_fsNotifier.reset(new FileSystemNotifier());
		_fsNotifierListener.reset(new FileSystemNotifierListener(this));
		_fsNotifier->AddListener(_fsNotifierListener);

		_thread = MakeThread("fsMonitor", std::bind(&FsMonitor::ThreadFunc, this, std::placeholders::_1));

		s_logger.Info() << "Created, rootPath: " << rootPath;
	}


	FsMonitor::~FsMonitor()
	{
		s_logger.Info() << "Destroying";
		_thread.reset();
	}


	void FsMonitor::ThreadFunc(const cancellation_token& t)
	{
		OperationProfiler p(s_logger, "Files scan", t);
		ScanFunc(_rootPath, t);
	}

	void FsMonitor::ScanFunc(const boost::filesystem::path& p, const cancellation_token& t)
	{
		using namespace boost;
		using namespace boost::filesystem;

		if (!t)
			return;

		if (find_if(_skipList, [&p](const regex& re) { smatch m; return regex_match(p.string(), m, re); }) != _skipList.end())
			return;

		if (is_directory(p))
		{
			_fsNotifier->AddPath(p.string());
			directory_iterator end;
			for (directory_iterator it(p); it != end; ++it)
				ScanFunc(*it, t);
		}
		else
		{
			s_logger.Info() << p.string();
		}
	}

	void FsMonitor::OnFileSystemEvent(FileSystemNotifierTarget target, FileSystemNotifierEvent event, const std::string& path)
	{
		switch (event.GetRaw())
		{
		case FileSystemNotifierEvent::Added:
			ScanFunc(path, dummy_cancellation_token());
			break;
		case FileSystemNotifierEvent::Removed:
			switch(target.GetRaw())
			{
			case FileSystemNotifierTarget::File:
				s_logger.Info() << "Remove " << path;
				break;
			case FileSystemNotifierTarget::Directory:
				s_logger.Info() << "Remove dir " << path;
				break;
			}
			break;
		case FileSystemNotifierEvent::Modified:
			switch(target.GetRaw())
			{
			case FileSystemNotifierTarget::File:
				s_logger.Info() << "Modified " << path;
				break;
			case FileSystemNotifierTarget::Directory:
				s_logger.Warning() << __func__ << "target: " << target << ", event: " << event <<  " - Not implemented!";
				break;
			}
			break;
		}
	}

}}
