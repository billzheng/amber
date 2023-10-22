#pragma once

#include <boost/algorithm/string.hpp>
#include <iomanip>
#include <sstream>
#include <string>

namespace miye
{
namespace string_utils
{

constexpr static const char* defaultSpaces = " \r\n\t";

inline std::string& trimLeft(std::string& str,
                             const char* spaces = defaultSpaces)
{
    return str.erase(0, str.find_first_not_of(spaces));
}

inline std::string& trimRight(std::string& str,
                              const char* spaces = defaultSpaces)
{
    return str.erase(str.find_last_not_of(spaces) + 1);
}

inline std::string& trim(std::string& str, const char* spaces = defaultSpaces)
{
    return trimLeft(trimRight(str, spaces));
}

inline std::string trimLeft(const std::string& src,
                            const char* spaces = defaultSpaces)
{
    auto str = src;
    return trimLeft(src, spaces);
}

inline std::string trimRight(const std::string& src,
                             const char* spaces = defaultSpaces)
{
    auto str = src;
    return trimRight(src, spaces);
}

inline std::vector<std::string> split(const std::string& str,
                                      char delimiter = '.')
{
    std::vector<std::string> result;
    result.reserve(10);

    std::string::size_type i = 0;
    decltype(i) j = 0;

    for (j = str.find(delimiter); j != std::string::npos;
         j = str.find(delimiter, j))
    {
        result.push_back(str.substr(i, j - i));
        i = ++j;
    }

    if (i < str.length())
    {
        result.push_back(str.substr(i, str.length()));
    }

    return result;
}

inline bool startsWith(const std::string& str, const std::string& substr)
{
    return str.compare(0, substr.length(), substr) == 0;
}

inline bool startsWith(const std::string& str, char c)
{
    return !str.empty() && str.front() == c;
}

inline bool endsWith(const std::string& str, const std::string& substr)
{
    if (substr.size() > str.size())
    {
        return false;
    }

    return str.compare(
               str.length() - substr.length(), substr.length(), substr) == 0;
}

inline bool endsWith(const std::string& str, char c)
{
    return !str.empty() && str.back() == c;
}

inline std::string& toLowercase(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

inline std::string& toUppercase(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
    return str;
}

template <typename T, uint32_t N>
inline uint32_t getArrayLength(const T (&arr)[N])
{
    return N;
}

template <uint32_t N>
inline void toRightPaddedString(char (&dest)[N], const std::string& from,
                                char padding = ' ')
{
    auto len = from.length();
    const size_t strLen = N > len ? len : N;

    for (size_t i = 0; i < strLen; ++i)
    {
        dest[i] = from[i];
    }

    for (size_t i = strLen; i < N; ++i)
    {
        dest[i] = padding;
    }
}

// performance to be enhanced
template <typename T>
inline std::string toPaddedString(T const& t, int len, char pad_char = '0')
{
    try
    {
        std::stringstream ss;
        ss << std::setw(len) << std::setfill(pad_char) << t;
        return ss.str();
    }
    catch (std::exception const& e)
    {
    }
    return std::string("");
}

template <size_t N>
inline void fillArray(char (&dest)[N], const std::string& from)
{
    auto len = from.length();
    const size_t strLen = N > len ? len : N;

    for (size_t i = 0; i < strLen; ++i)
    {
        dest[i] = from[i];
    }
}

inline char** toCStringArray(const std::vector<std::string>& data)
{
    const size_t len = data.size();

    char** instrumentsArr = new char*[len]();

    for (size_t i = 0; i < len; ++i)
    {
        instrumentsArr[i] = new char[data[i].size() + 1]();
        std::strncpy(
            instrumentsArr[i], data[i].c_str(), sizeof(instrumentsArr) - 1);
    }

    return instrumentsArr;
}

inline void releaseCStringArray(char** data, size_t len)
{
    for (size_t i = 0; i < len; ++i)
    {
        delete[] data[i];
    }

    delete[] data;
    data = nullptr;
}

inline std::string join(const std::vector<std::string>& data,
                        char delimiter = ';')
{
    std::stringstream ss;
    for (auto&& str : data)
    {
        ss << str << delimiter;
    }
    return ss.str();
}

} // namespace string_utils
} // namespace miye
