#include <hide/Buffer.h>


namespace hide
{

	Buffer::Buffer(const std::string& name)
		: _name(name)
	{
	}


	Buffer::~Buffer()
	{
	}


	void Buffer::InsertText(const Location& location, const std::string& text)
	{
	}


	void Buffer::ReplaceText(const Location& location, const std::string& newText, size_t oldTextSize)
	{
	}


	void Buffer::RemoveText(const Location& location, size_t textSize)
	{
	}

}
