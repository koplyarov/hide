#include <hide/utils/ReadBufferLinesListener.h>


namespace hide
{

	HIDE_NAMED_LOGGER(ReadBufferLinesListener);

	ReadBufferLinesListener::ReadBufferLinesListener(const CallbackFunc& callback)
		: _callback(callback), _ofs(0)
	{ }


	void ReadBufferLinesListener::OnBufferChanged(const IReadBuffer& buf)
	{
		ByteArray data = buf.Read(_ofs);

		auto new_line_it = data.begin();
		do {
			auto next_new_line_it = std::find(new_line_it, data.end(), '\n');
			std::copy(new_line_it, next_new_line_it, std::back_inserter(_accumStr));

			if (next_new_line_it != data.end())
			{
				++next_new_line_it;
				_callback(_accumStr);
				_accumStr.clear();
			}

			new_line_it = next_new_line_it;
		}
		while (new_line_it != data.end());

		_ofs += data.size();
	}

}
