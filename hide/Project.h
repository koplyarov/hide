#ifndef HIDE_HIDE_H
#define HIDE_HIDE_H


#include <map>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include <hide/Buffer.h>
#include <hide/IBuildSystem.h>
#include <hide/ILanguagePlugin.h>
#include <hide/Indexer.h>
#include <hide/utils/NamedLogger.h>
#include <hide/utils/Utils.h>


namespace hide
{

	class Project;
	HIDE_DECLARE_PTR(Project);

	class Project
	{
		HIDE_NONCOPYABLE(Project);

		typedef std::map<std::string, BufferPtr>	BuffersMap;

	public:
		typedef std::vector<IFilePtr>				FilesVector;

	private:
		static NamedLogger			s_logger;
		IBuildSystemProberPtrArray	_buildSystemProbers;
		ILanguagePluginPtrArray		_langPlugins;
		IBuildSystemPtr				_currentBuildSystem;
		FilesVector					_files;
		BuffersMap					_buffers;
		IndexerPtr					_indexer;

	public:
		Project();
		~Project();

		IBuildSystemPtr GetBuildSystem();
		IndexerPtr GetIndexer();

		void AddBuffer(const BufferPtr& buffer);
		void RemoveBuffer(const std::string& bufferName);

		IFilePtr GetFileByPath(const std::string& filepath);
		FilesVector GetFiles() const { return _files; }

		static ProjectPtr CreateAuto(const StringArray& skipRegexesList);

	private:
		void ScanProjectFunc(const boost::filesystem::path& p, const std::vector<boost::regex>& skipList, const std::string& indent = "");
	};

}

#endif
