#ifndef HIDE_FS_FSMONITOR_H
#define HIDE_FS_FSMONITOR_H


#include <hide/utils/FileSystemNotifier.h>
#include <hide/utils/NamedLogger.h>
#include <hide/utils/rethread.h>

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include <vector>


namespace hide {
namespace fs
{

	class FsMonitor
	{
		class FileSystemNotifierListener;

		typedef std::vector<boost::regex>			RegexesVector;

	private:
		static NamedLogger					s_logger;
		boost::filesystem::path				_rootPath;
		RegexesVector						_skipList;
		thread								_thread;
		IFileSysterNotifierListenerPtr		_fsNotifierListener;
		FileSystemNotifierPtr				_fsNotifier;

	public:
		FsMonitor(boost::filesystem::path rootPath, const RegexesVector& skipList);
		~FsMonitor();

	private:
		void ThreadFunc(const cancellation_token& t);

		void ScanFunc(const boost::filesystem::path& p, const cancellation_token& t);
		void OnFileSystemEvent(FileSystemNotifierTarget target, FileSystemNotifierEvent event, const std::string& path);
	};
	HIDE_DECLARE_PTR(FsMonitor);

}}

#endif
