#ifndef HIDE_BUFFER_H
#define HIDE_BUFFER_H


#include <hide/ILanguagePlugin.h>
#include <hide/Utils.h>


namespace hide
{

	class Buffer
	{
		HIDE_NONCOPYABLE(Buffer);

	private:
		std::string				_name;
		ILanguagePluginPtr		_langPlugin;

	public:
		Buffer(const std::string& name);
		~Buffer();

		std::string GetName() const { return _name; }
	};
	HIDE_DECLARE_PTR(Buffer);

}

#endif
