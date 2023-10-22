#pragma once
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <string.h>
#include <string>
#include <type_traits>

namespace miye::trading::fix
{

template <typename T, typename = std::enable_if_t<std::is_integral<T>::value && std::is_unsigned<T>::value>>
inline T toUInteger(const char* p, size_t len) noexcept
{
    static_assert(!std::is_signed<T>::value, "T must be unsigned");
    T v{};

    for (size_t i = 0; i < len; ++i)
    {
        v *= 10;
        v += (p[i] - '0');
    }

    return v;
}

template <typename T, typename = std::enable_if_t<std::is_integral<T>::value && std::is_signed<T>::value>>
inline T toInteger(const char* p, size_t len) noexcept
{
    static_assert(std::is_signed<T>::value, "T must be signed");

    T v{};

    int32_t sign = (*p == '-') ? -1 : 1;
    size_t i     = (sign == -1) ? 1 : 0;
    for (; i < len; ++i)
    {
        v *= 10;
        v += (p[i] - '0');
    }

    return v * sign;
}

template <typename T, typename = std::enable_if_t<std::is_integral<T>::value && std::is_unsigned<T>::value>>
inline T toUInteger(const char* p) noexcept
{
    static_assert(!std::is_signed<T>::value, "T must be unsigned");

    T val{};

    while (*p)
    {
        val *= 10;
        val += *p++ - '0';
    }

    return val;
}

template <typename T, typename = std::enable_if_t<std::is_integral<T>::value && std::is_signed<T>::value>>
inline T toInteger(const char* p) noexcept
{
    static_assert(std::is_signed<T>::value, "T must be signed");
    T val{};

    int32_t neg = (*p == '-') ? -1 : 1;
    if (neg == -1)
    {
        p++;
    }

    while (*p)
    {
        val *= 10;
        val += *p++ - '0';
    }

    return val * neg;
}

template <typename T, typename = std::enable_if_t<std::is_integral<T>::value && std::is_unsigned<T>::value>>
inline T toUInteger(const std::string& p) noexcept
{
    static_assert(!std::is_signed<T>::value, "T must be unsigned");
    return toUInteger<T>(p.data(), p.size());
}

template <typename T, typename = std::enable_if_t<std::is_integral<T>::value && std::is_signed<T>::value>>
inline T toInteger(const std::string& p) noexcept
{
    static_assert(std::is_signed<T>::value, "T must be signed");
    return toInteger<T>(p.data(), p.size());
}

constexpr inline uint32_t calcCheckSum(const char* p, size_t len) noexcept
{
    uint32_t checkSum{};

    for (size_t i = 0; i < len; ++i)
    {
        checkSum += p[i];
    }

    return checkSum;
}

template <size_t N>
constexpr inline uint32_t calcCheckSum(const char (&arr)[N]) noexcept
{
    uint32_t checkSum{};

    for (size_t i = 0; i < N; ++i)
    {
        checkSum += arr[i];
    }

    return checkSum;
}

inline uint32_t mod(uint32_t num) noexcept
{
    auto const v = num;
    char c       = *(char*)(&v);
    return c;
}

/*
 * naive version of integer to string
 */
inline size_t u32toa(uint32_t value, char* buffer)
{
    char temp[11]{}; // enough to hold unit32_t
    char* p = temp;

    do
    {
        *p++ = char(value % 10) + '0';
        value /= 10;
    } while (value > 0);

    size_t len = p - temp;

    do
    {
        *buffer++ = *--p;
    } while (p != temp);

    return len;
}

inline size_t i32toa(int32_t value, char* buffer)
{
    size_t len = 0;
    uint32_t u = static_cast<uint32_t>(value);
    if (value < 0)
    {
        *buffer++ = '-';
        u         = ~u + 1;
        len++;
    }
    return len + u32toa(u, buffer);
}

inline size_t u64toa(uint64_t value, char* buffer)
{
    char temp[20]{};
    char* p = temp;
    do
    {
        *p++ = char(value % 10) + '0';
        value /= 10;
    } while (value > 0);

    size_t len = p - temp;

    do
    {
        *buffer++ = *--p;
    } while (p != temp);

    return len;
}

inline size_t i64toa(int64_t value, char* buffer)
{
    size_t len = 0;
    uint64_t u = static_cast<uint64_t>(value);
    if (value < 0)
    {
        *buffer++ = '-';
        u         = ~u + 1;
        len++;
    }
    return len + u64toa(u, buffer);
}

/*
 * naive version of string to double
 */
template <typename T>
inline T toFloatingPoint(const char* p, size_t len)
{
    const char* begin(p);
    const char* end(p + len);

    bool neg = false;
    if (*begin == '-')
    {
        neg = true;
        ++begin;
    }

    T v{0.0};
    while (*begin >= '0' && *begin <= '9' && begin < end)
    {
        v = (v * 10.0) + (*begin - '0');
        ++begin;
    }

    if (*begin == '.')
    {
        double f{0.0};
        int n{0};
        ++begin;
        while (*begin >= '0' && *begin <= '9' && begin < end)
        {
            f = (f * 10.0) + (*begin - '0');
            ++begin;
            ++n;
        }
        v += f / std::pow(10.0, n);
    }
    if (neg)
    {
        v = -v;
    }
    return v;
}

} // namespace miye::trading::fix
