/*
 * stringutils.cpp
 *
 * Purpose: string utilities
 *
 * Author: 
 */
#include "libcore/utils/stringutils.hpp"

namespace miye
{
namespace utils
{

std::vector<std::string> split(const std::string& s,
                               const std::string& separator)
{
    std::vector<std::string> output;

    std::string::size_type prev_pos = 0, pos = 0;

    while ((pos = s.find(separator, pos)) != std::string::npos)
    {
        output.push_back(s.substr(prev_pos, pos - prev_pos));

        prev_pos = ++pos;
    }

    output.push_back(s.substr(prev_pos, pos - prev_pos));

    return output;
}

bool expect_bool(std::string const& rhs)
{

    if (!strcmp(rhs.c_str(), "1"))
    {
        return true;
    }
    else if (!strcmp(rhs.c_str(), "0"))
    {
        return false;
    }
    else if (icasecmp(rhs, "true"))
    {
        return true;
    }
    else if (icasecmp(rhs, "false"))
    {
        return false;
    }
    else
        INVARIANT_FAIL(
            "Boolean element must have value [true|1|false|0]. Supplied = "
            << rhs);
}

uint64_t quick_hash(const char* s, uint64_t seed)
{
    uint64_t hash = seed;
    while (*s)
        hash = hash * 101 + *s++;
    return hash;
}

std::string base64_encode(const std::string& in)
{

    std::string out;

    int val = 0, valb = -6;
    for (uchar c : in)
    {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0)
        {
            out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
                          "0123456789+/"[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6)
        out.push_back(
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
                [((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4)
        out.push_back('=');
    return out;
}

std::string base64_decode(const std::string& in)
{

    std::string out;

    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++)
        T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
              [i]] = i;

    int val = 0, valb = -8;
    for (uchar c : in)
    {
        if (T[c] == -1)
            break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0)
        {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

} // namespace utils
} // namespace miye
