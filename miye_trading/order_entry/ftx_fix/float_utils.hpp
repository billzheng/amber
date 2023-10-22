#pragma once
#include <cstdint>

namespace qx
{
namespace float_utils
{

template <typename T>
T constrain(T max_value, T min_value, T value)
{
    if (max_value && value > max_value)
    {
        value = max_value;
    }
    else if (min_value && value < min_value)
    {
        value = min_value;
    }
    return value;
}

template <typename T>
T constrain(T max_value, T min_value, T value, bool min_only)
{
    if (min_only)
    {
        if (max_value && value > max_value)
        {
            value = max_value;
        }
    }
    else
    {
        if (max_value && value > max_value)
        {
            value = max_value;
        }
        else if (min_value && value < min_value)
        {
            value = min_value;
        }
    }
    return value;
}

template <typename T>
T divide_or_zero(T numerator, T denominator)
{
    if (denominator != T(0.0))
        return numerator / denominator;
    return 0;
}

// The following definitions are from The art of computer programming by Knuth.
inline bool approximatelyEqual(float a, float b, float epsilon)
{
    return fabs(a - b) <= ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}

inline bool essentiallyEqual(float a, float b, float epsilon)
{
    return fabs(a - b) <= ((fabs(a) > fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}

inline bool definitelyGreaterThan(float a, float b, float epsilon)
{
    return (a - b) > ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}

inline bool definitelyLessThan(float a, float b, float epsilon)
{
    return (b - a) > ((fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * epsilon);
}

} // namespace float_utils
} // namespace qx
