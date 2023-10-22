/*
 * stringutils.hpp
 *
 * Purpose: string utilities
 *
 * Author: 
 */
#pragma once

#include "libcore/essential/assert.hpp"
#include <iomanip>
#include <iostream>
#include <string.h>
#include <string>

namespace miye
{
namespace utils
{

inline unsigned char decode_hex_char(char const* str)
{
    return *str > '9' ? (tolower(*str) - 'a' + 10) : *str - '0';
}

struct escaped_string
{
    explicit escaped_string(char const* str) : s(str), len(strlen(str))
    {
    }
    explicit escaped_string(char const* str, size_t l) : s(str), len(l)
    {
    }
    explicit escaped_string(std::string const& str)
        : s(str.c_str()), len(str.size())
    {
    }

    char const* s;
    size_t len;
};

template <typename OStream>
OStream& operator<<(OStream& out, escaped_string const& s)
{
    static const char digit[] = "0123456789abcdef";
    char const* str = s.s;
    char const* end = s.s + s.len;
    while (str != end)
    {
        switch (*str)
        {
        case '\b':
        case '\f':
        case '\n':
        case '\r':
        case '\t':
        case '"':
        case '\\':
        case '/': {
            unsigned char c = *str;
            out << "\\\\x" << digit[c >> 4] << digit[c & 0xf];
            break;
        }
        default:
            if (isprint(*str))
                out << *str;
            else
            {
                unsigned char c = *str;
                out << "\\\\x" << digit[c >> 4] << digit[c & 0xf];
            }
            break;
        }
        ++str;
    }
    return out;
}

struct unescaped_string
{
    explicit unescaped_string(char const* str) : s(str), len(strlen(str))
    {
    }
    explicit unescaped_string(char const* str, size_t l) : s(str), len(l)
    {
    }
    explicit unescaped_string(std::string const& str)
        : s(str.c_str()), len(str.size())
    {
    }

    char const* s;
    size_t len;
};

template <typename OStream>
OStream& operator<<(OStream& out, unescaped_string const& s)
{
    char const* str = s.s;
    char const* end = s.s + s.len;
    while (str != end)
    {
        if (*str == '\\')
        {
            ++str;
            switch (*str)
            {
            case 'b':
                out << '\b';
                break;
            case 'f':
                out << '\f';
                break;
            case 'n':
                out << '\n';
                break;
            case 'r':
                out << '\r';
                break;
            case 't':
                out << '\t';
                break;
            case '"':
                out << '"';
                break;
            case '\\':
                if (str + 1 != end && *(str + 1) == 'x')
                {
                    ++str;
                    unsigned char c = decode_hex_char(++str) << 4;
                    c |= decode_hex_char(++str);
                    out << c;
                    break;
                }
                out << '\\';
                break;
            case '/':
                out << '/';
                break;
            case 'x': {
                unsigned char c = decode_hex_char(++str) << 4;
                c |= decode_hex_char(++str);
                out << c;
                break;
            }
            case 'u': {
                str += 2;
                unsigned char c = decode_hex_char(++str) << 4;
                c |= decode_hex_char(++str);
                out << c;
                break;
            }
            default:
                ASSERT(false);
                break;
            }
            ++str;
        }
        else
            out << *str++;
    }
    return out;
}

struct hex_string
{
    explicit hex_string(char const* str) : s(str), len(strlen(str))
    {
    }
    explicit hex_string(char const* str, size_t l) : s(str), len(l)
    {
    }
    explicit hex_string(std::string const& str)
        : s(str.c_str()), len(str.size())
    {
    }

    char const* s;
    size_t len;
};
template <typename OStream>
OStream& operator<<(OStream& out, hex_string const& s)
{
    int bytenum = 0;
    char const* ptr = s.s;
    auto end = s.s + s.len;
    out << "\n";
    while (ptr < end)
    {
        out << std::hex << std::setfill('0') << std::setw(8) << bytenum << ":";
        char raw[17];
        raw[16] = 0;
        for (int i = 0; i != 8; ++i)
        {
            if (ptr == end)
            {
                break;
            }
            out << " ";
            out << std::hex << (uint16_t)*ptr;
            if (isprint(*ptr))
            {
                raw[i * 2] = *ptr;
            }
            else
            {
                raw[i * 2] = '.';
            }
            ++ptr;
            if (ptr == end)
            {
                out << "  ";
                break;
            }
            out << std::hex << (uint16_t)*ptr;
            if (isprint(*ptr))
            {
                raw[i * 2 + 1] = *ptr;
            }
            else
            {
                raw[i * 2 + 1] = '.';
            }
            ++ptr;
        }

        out << "  |" << raw << "|" << std::endl;
        bytenum += 0x10;
    }
    return out;
}

std::vector<std::string> split(const std::string& s,
                               const std::string& delimiter);

inline std::string strip(std::string const& str)
{
    int first_non_ws = 0;
    while (first_non_ws < int(str.size()) && isspace(str[first_non_ws]))
        ++first_non_ws;
    int last_non_ws = int(str.size()) - 1;
    while (last_non_ws >= 0 && isspace(str[last_non_ws]))
        --last_non_ws;

    if (last_non_ws >= first_non_ws && str.length() > 0)
        return str.substr(first_non_ws, last_non_ws - first_non_ws + 1);
    return std::string();
}

inline bool icasecmp(const std::string& l, const std::string& r)
{
    return l.size() == r.size() &&
           equal(l.cbegin(),
                 l.cend(),
                 r.cbegin(),
                 [](std::string::value_type l1, std::string::value_type r1) {
                     return toupper(l1) == toupper(r1);
                 });
}

inline void replace(std::string& str, const std::string& oldStr,
                    const std::string& newStr)
{
    std::string::size_type pos = 0u;
    while ((pos = str.find(oldStr, pos)) != std::string::npos)
    {
        str.replace(pos, oldStr.length(), newStr);
        pos += newStr.length();
    }
}

inline void truncate_from(std::string& str, const std::string& oldStr)
{
    std::string::size_type pos = str.find(oldStr);
    if (pos != std::string::npos)
        str.replace(pos, str.length() - pos, "");
}

inline void truncate_to(std::string& str, const std::string& oldStr)
{
    std::string::size_type pos = str.find(oldStr);
    if (pos != std::string::npos)
        str.replace(0, pos + 1, "");
}

inline std::string first_numberstring(std::string const& str)
{
    std::size_t const n = str.find_first_of("0123456789");
    if (n != std::string::npos)
    {
        std::size_t const m = str.find_first_not_of("0123456789", n);
        return str.substr(n, m != std::string::npos ? m - n : m);
    }
    return std::string();
}

bool expect_bool(std::string const& rhs);

template <typename T>
T expect_whole_number(std::string const& rhs)
{
    T t;
    t = ::strtol(rhs.c_str(), NULL, 10);
    return t;
}

template <typename T>
T expect_whole_number(std::string& rhs)
{
    T t;
    t = ::strtol(rhs.c_str(), NULL, 10);
    return t;
}

template <typename T>
T expect_whole_unsigned_number(std::string const& rhs)
{
    T t;
    t = ::strtoul(rhs.c_str(), NULL, 10);
    return t;
}

template <typename T>
T expect_whole_unsigned_number(std::string& rhs)
{
    T t;
    t = ::strtoul(rhs.c_str(), NULL, 10);
    return t;
}

template <typename T>
T expect_real_number(std::string const& rhs)
{
    T t;
    std::istringstream iss(rhs);
    iss >> t >> std::ws;
    return t;
}

template <typename T>
T expect_real_number(std::string& rhs)
{
    T t;
    std::istringstream iss(rhs);
    iss >> t >> std::ws;
    return t;
}

uint64_t quick_hash(const char* s, uint64_t seed);

typedef unsigned char uchar;

std::string base64_encode(const std::string& in);
std::string base64_decode(const std::string& in);
} // namespace utils
} // namespace miye
