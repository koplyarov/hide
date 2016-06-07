#ifndef HIDE_HIDE_H
#define HIDE_HIDE_H


#include <hide/Buffer.h>
#include <hide/ContextUnawareSyntaxHighlighter.h>
#include <hide/IBuildSystem.h>
#include <hide/ILanguagePlugin.h>
#include <hide/Indexer.h>
#include <hide/ProjectFiles.h>
#include <hide/fs/FsMonitor.h>
#include <hide/utils/FileSystemNotifier.h>
#include <hide/utils/NamedLogger.h>
#include <hide/utils/Utils.h>

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include <map>
#include <string>


namespace hide
{

	class Project;
	HIDE_DECLARE_PTR(Project);

	class Project
	{
		HIDE_NONCOPYABLE(Project);

		typedef std::map<std::string, BufferPtr>	BuffersMap;

		class FileSystemNotifierListener;

	public:
		typedef std::vector<IFilePtr>				FilesVector;
		typedef std::vector<boost::regex>			RegexesVector;

	private:
		static NamedLogger					s_logger;

		RegexesVector						_skipList;
		fs::FsMonitorPtr					_fsMonitor;
		IBuildSystemProberPtrArray			_buildSystemProbers;
		ILanguagePluginPtrArray				_langPlugins;
		IBuildSystemPtr						_currentBuildSystem;
		ProjectFilesPtr						_files;
		IFileSysterNotifierListenerPtr		_fsNotifierListener;
		FileSystemNotifierPtr				_fsNotifier;

		BuffersMap							_buffers;
		IndexerPtr							_indexer;
		ContextUnawareSyntaxHighlighterPtr	_contextUnawareSyntaxHighlighter;

	public:
		Project(const RegexesVector& skipList);
		~Project();

		IBuildSystemPtr GetBuildSystem();
		IndexerPtr GetIndexer();
		ContextUnawareSyntaxHighlighterPtr GetContextUnawareSyntaxHighlighter();

		void AddBuffer(const BufferPtr& buffer);
		void RemoveBuffer(const std::string& bufferName);

		IFilePtr GetFileByPath(const std::string& filepath);
		FilesVector GetFiles() const { return _files->GetFiles(); }

		static ProjectPtr CreateAuto(const StringArray& skipRegexesList);

	private:
		void ScanProjectFunc(const boost::filesystem::path& p);
		void OnFileSystemEvent(FileSystemNotifierTarget target, FileSystemNotifierEvent event, const std::string& path);
	};

}

#endif
