#include <hide/Project.h>


namespace hide
{

	Project::Project()
	{
	}


	Project::~Project()
	{
	}


    void Project::AddBuffer(const BufferPtr& buffer)
    {
		std::string name = buffer->GetName();
        HIDE_CHECK(_buffers.find(name) == _buffers.end(), std::runtime_error("Buffer " + name + " already registered!"));
        _buffers.insert(std::make_pair(name, buffer));
    }


    void Project::RemoveBuffer(const std::string& bufferName)
    {
        _buffers.erase(bufferName);
    }


	std::string Project::GetLanguageName() const
	{
		return "test";
	}

}
