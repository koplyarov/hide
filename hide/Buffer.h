#ifndef HIDE_BUFFER_H
#define HIDE_BUFFER_H


#include <hide/ILanguagePlugin.h>
#include <hide/Location.h>
#include <hide/utils/Utils.h>


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

		void InsertText(const Location& location, const std::string& text);
		void ReplaceText(const Location& location, const std::string& newText, size_t oldTextSize);
		void RemoveText(const Location& location, size_t textSize);
	};
	HIDE_DECLARE_PTR(Buffer);

}

#endif
