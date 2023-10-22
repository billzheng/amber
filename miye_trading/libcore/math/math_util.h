#pragma once
#include <cmath>
#include <stdint.h>

namespace miye
{
namespace trading
{
namespace math
{

#define EPOSILON 0.0000000001

template <typename T>
inline bool floatEqual(T a, T b) noexcept
{
    return std::isless(std::fabs(a - b), EPOSILON);
}

template <typename T>
bool lessThan(T a, T b) noexcept
{
    if (!floatEqual(a, b))
    {
        return std::isless(a, b);
    }
    return false;
}

template <typename T>
bool greaterThan(T a, T b) noexcept
{
    if (!floatEqual(a, b))
    {
        return std::isgreater(a, b);
    }
    return false;
}

template <typename T>
bool lessEqual(T a, T b) noexcept
{
    if (floatEqual(a, b))
    {
        return true;
    }
    return std::isless(a, b);
}

template <typename T>
bool greaterEqual(T a, T b) noexcept
{
    if (floatEqual(a, b))
    {
        return true;
    }
    return std::isgreater(a, b);
}

} // namespace math
} // namespace trading
} // namespace miye
