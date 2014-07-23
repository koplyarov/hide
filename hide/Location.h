#ifndef HIDE_LOCATION_H
#define HIDE_LOCATION_H


#include <string>


namespace hide
{

	class Location
	{
	private:
    	std::string		_filename;
        size_t			_line;
        size_t			_column;

	public:
	    Location()
        	: _filename(), _line(0), _column(0)
        { }

	    Location(const std::string& filename, size_t line, size_t column)
        	: _filename(filename), _line(line), _column(column)
        { }

        bool IsValid() const { return !_filename.empty(); }

        std::string GetFilename() const	{ return _filename; }
        size_t GetLine() const			{ return _line; }
        size_t GetColumn() const		{ return _column; }
	};

}

#endif