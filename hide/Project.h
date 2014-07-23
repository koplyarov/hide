#ifndef HIDE_HIDE_H
#define HIDE_HIDE_H


#include <map>
#include <string>

#include <hide/Buffer.h>
#include <hide/Utils.h>


namespace hide
{

	class Project
	{
		HIDE_NONCOPYABLE(Project);

		typedef std::map<std::string, BufferPtr>	BuffersMap;

	private:
		BuffersMap		_buffers;

	public:
		Project();
		~Project();

		void AddBuffer(const BufferPtr& buffer);
		void RemoveBuffer(const std::string& bufferName);

		std::string GetLanguageName() const;
	};

}

#endif
