/*
 * csv.hpp
 *
 * Purpose: CSV parsing
 * Author: 
 */

#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <vector>

using std::vector;

namespace miye { namespace parsing {


struct csv_header_generator
{
    csv_header_generator()
    : sep('\0')
    {}

    template<typename T>
    void visit(char const* name, T const& t)
    {
        if (sep != '\0')
        {
            header << sep;
        }
        header << name;
        sep = ',';
    }

    std::ostringstream header;
    char sep;
};

struct csv_line_generator
{
    csv_line_generator()
    : sep('\0')
    {}

    template<typename T>
    void visit(char const* name, T const& t)
    {
        if (sep != '\0')
        {
            line << sep;
        }
        line << t;
        sep = ',';
    }

    std::ostringstream line;
    char sep;
};

template<char Delim>
class csv_line_t
{
    public:
        void next_line(std::istream& iss)
        {
            std::string line;
            std::getline(iss,line);

            std::stringstream   line_stream(line);
            std::string         column;

            m_data.clear();
            while(std::getline(line_stream, column, Delim))
            {
                m_data.push_back(column);
            }
        }

        std::string const& operator[](std::size_t index) const
        {
            return m_data[index];
        }

        size_t size() const
        {
            return m_data.size();
        }
    private:
        vector<std::string>    m_data;
};

template<char Delim>
std::istream& operator>>(std::istream& iss, csv_line_t<Delim>& csv_line)
{
    csv_line.next_line(iss);
    return iss;
}

template<typename OStream, char Delim>
OStream& operator<<(OStream& os, csv_line_t<Delim> const& s)
{
    for (size_t i = 0; i < s.size(); ++i)
    {
        os << s[i];
        if (i < s.size() - 1)
            os << ",";
    }
    return os;
}

}}
