#include <hide/utils/PipeLinesReader.h>


namespace hide
{

	HIDE_NAMED_LOGGER(PipeLinesReader);

	PipeLinesReader::PipeLinesReader(const BufferCallbackFunc& bufferChangedCallback)
		: _bufferChangedCallback(bufferChangedCallback)
	{ }


	void PipeLinesReader::OnData(const ByteArray& data)
	{
		auto new_line_it = data.begin();
		do {
			auto next_new_line_it = std::find(new_line_it, data.end(), '\n');
			std::copy(new_line_it, next_new_line_it, std::back_inserter(_accumStr));

			if (next_new_line_it != data.end())
			{
				++next_new_line_it;
				_bufferChangedCallback(_accumStr);
				_accumStr.clear();
			}

			new_line_it = next_new_line_it;
		}
		while (new_line_it != data.end());
	}


	void PipeLinesReader::OnEndOfData()
	{
		if (!_accumStr.empty())
			_bufferChangedCallback(_accumStr);
	}

}
