#ifndef HIDE_LOCATION_H
#define HIDE_LOCATION_H


#include <iostream>
#include <string>

#include <boost/lexical_cast.hpp>

#include <hide/utils/MembersVisitor.h>
#include <hide/utils/Utils.h>


namespace hide
{

	class Location
	{
	private:
    	std::string		_filename;
        uint32_t		_line;
        uint32_t		_column;

	public:
	    Location()
        	: _filename(), _line(0), _column(0)
        { }

		Location(const std::string& filename, uint32_t line, uint32_t column)
			: _filename(filename), _line(line), _column(column)
        { }

        bool IsValid() const { return !_filename.empty(); }

        std::string GetFilename() const	{ return _filename; }
        uint32_t GetLine() const		{ return _line; }
        uint32_t GetColumn() const		{ return _column; }

		HIDE_DECLARE_TO_STRING_METHOD()
		HIDE_DECLARE_MEMBERS("filename", &Location::_filename, "line", &Location::_line, "column", &Location::_column)
	};

	HIDE_DECLARE_WRITE_TO_OSTREAM(Location, s << v.GetFilename() << ':' << (v.GetLine()) << ':' << (v.GetColumn()); )

}

#endif
