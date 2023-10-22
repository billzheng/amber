/*
 * mathutils.hpp
 *
 * Purpose: math utility functions
 *
 */

#pragma once

#include <assert.h>
#include <cmath>
#include <limits.h>

namespace miye
{
namespace math
{

// 10 decimal points
#define EPOSILON 0.00000000001

template <class T>
inline T sign(const T& a)
{
    return a >= 0 ? 1 : -1;
}

template <typename T>
inline bool floatEqual(T a, T b)
{
    return std::islessequal(std::fabs(a - b), EPOSILON);
}

template <typename T>
inline bool lessThan(T a, T b)
{
    if (!floatEqual(a, b))
    {

        return std::isless(a, b);
    }
    return false;
}

template <typename T>
inline bool greaterThan(T a, T b)
{
    if (!floatEqual(a, b))
    {
        return std::isgreater(a, b);
    }
    return false;
}

template <typename T>
inline bool lessEqual(T a, T b)
{
    if (floatEqual(a, b))
    {
        return true;
    }

    return std::isless(a, b);
}

template <typename T>
inline bool greaterEqual(T a, T b)
{
    if (floatEqual(a, b))
    {
        return true;
    }

    return std::isgreater(a, b);
}

/*
template <typename T>
inline bool fp_eq(T a, T b, T eps = std::numeric_limits<T>::min())
{
    return math::fast_fabs(a - b) < eps;
}

template <typename T>
inline bool fp_lt(T a, T b, T eps = std::numeric_limits<T>::min())
{
    return (a - b) < -eps;
}

template <typename T>
inline bool fp_le(T a, T b, T eps = std::numeric_limits<T>::min())
{
    return (a - b) < eps;
}

template <typename T>
inline bool fp_gt(T a, T b, T eps = std::numeric_limits<T>::min())
{
    return (a - b) > eps;
}

template <typename T>
inline bool fp_ge(T a, T b, T eps = std::numeric_limits<T>::min())
{
    return (a - b) > -eps;
}

namespace utils
{

constexpr static const double EPSILON = 0.0000001;

template <typename T> inline bool approximatelyEqual(T a, T b, T epsilon)
{
    return math::fast_fabs(a - b) <=
           ((math::fast_fabs(a) < math::fast_fabs(b) ? math::fast_fabs(b)
                                                     : math::fast_fabs(a)) *
            epsilon);
}

constexpr inline int getPow(int base, int exp) noexcept
{
    assert(exp > 0);

    auto result = 1;
    for (int i = 0; i < exp; ++i)
    {
        result *= base;
    }

    return result;
}

constexpr inline double getNegPow(int base, int exp) noexcept
{
    return 1.0 / getPow(base, exp ^ ((exp ^ -exp) & -(exp > 0)));
}

constexpr inline float to_nearest_tick(float price, float ticksize)
{
    return (long)((price + ticksize / 2.0f) / ticksize) * ticksize;
}

constexpr inline double to_nearest_tick(double price, double ticksize)
{
    return (long)((price + ticksize / 2.0) / ticksize) * ticksize;
}
*/

} // namespace math
} // namespace miye
