#include <hide/Hide.h>


namespace hide
{

	Hide::Hide()
	{
	}


	Hide::~Hide()
	{
	}


    void Hide::AddBuffer(const BufferPtr& buffer)
    {
		std::string name = buffer->GetName();
        HIDE_CHECK(_buffers.find(name) == _buffers.end(), std::runtime_error("Buffer " + name + " already registered!"));
        _buffers.insert(std::make_pair(name, buffer));
    }


    void Hide::RemoveBuffer(const std::string& bufferName)
    {
        _buffers.erase(bufferName);
    }


	std::string Hide::GetLanguageName() const
	{
		return "test";
	}

}
