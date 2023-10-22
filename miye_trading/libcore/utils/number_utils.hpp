#pragma once

#include <iostream>
#include <sstream>
#include <string>

namespace miye
{
namespace number_utils
{

template <typename T, typename U>
inline T toInt(const U& t)
{
    std::stringstream ss;
    ss << t;
    T val{};
    ss >> val;
    return val;
}

template <typename T,
          typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
inline T toInt(const std::string& data)
{
    T val{};
    size_t i = 0;
    bool isNegative{false};

    if (!data.empty() && data.front() == '-')
    {
        isNegative = true;
        ++i;
    }

    for (; i < data.size(); ++i)
    {
        val *= 10;
        val += data[i] - '0';
    }

    if (isNegative)
    {
        val *= -1;
    }
    return val;
}

inline double toDouble(const std::string& data)
{
    char* end;
    return std::strtod(data.c_str(), &end);
}

inline bool onlyContainsNumber(const std::string& str)
{
    for (auto const c : str)
    {
        if (!std::isdigit(c))
        {
            return false;
        }
    }
    return true;
}

} // namespace number_utils
} // namespace miye
