#ifndef HIDE_HIDE_H
#define HIDE_HIDE_H


#include <map>
#include <string>

#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

#include <hide/Buffer.h>
#include <hide/ILanguagePlugin.h>
#include <hide/Utils.h>


namespace hide
{

	class Project;
	HIDE_DECLARE_PTR(Project);

	class Project
	{
		HIDE_NONCOPYABLE(Project);

		typedef std::vector<ILanguagePluginPtr>		LanguagePluginsVector;
		typedef std::map<std::string, BufferPtr>	BuffersMap;

	public:
		typedef std::vector<IFilePtr>				FilesVector;

	private:
		LanguagePluginsVector		_langPlugins;
		FilesVector					_files;
		BuffersMap					_buffers;

	public:
		Project();
		~Project();

		void AddBuffer(const BufferPtr& buffer);
		void RemoveBuffer(const std::string& bufferName);

		std::string GetLanguageName() const;

		FilesVector GetFiles() const { return _files; }

		static ProjectPtr CreateAuto(const std::vector<std::string>& skipRegexesList);

	private:
		void ScanProjectFunc(const boost::filesystem::path& p, const std::vector<boost::regex>& skipList, const std::string& indent = "");
	};

}

#endif
