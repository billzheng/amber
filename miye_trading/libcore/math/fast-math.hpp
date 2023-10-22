/*
 * fast-math.hpp
 *
 * Purpose: fast math functions, delegates to std or vdt as appropriate
 * Author: Seah
 *
 */

#pragma once

#if INCLUDE_VDT
#include <vdt/vdtMath.h>
#else
extern "C"
{
#include <math.h>
};
#endif

namespace miye
{
namespace math
{

inline float fast_ppow(float x, size_t p)
{
    float t = 1.0;
    for (size_t i = 0; i < p; ++i)
    {
        t *= x;
    }
    return t;
}

inline float fast_npow(float x, int p)
{
    float t = 1.0;
    for (int i = 0; i < -p; ++i)
    {
        t /= x;
    }
    return t;
}

inline double fast_ppow(double x, size_t p)
{
    double t = 1.0;
    for (size_t i = 0; i < p; ++i)
    {
        t *= x;
    }
    return t;
}

inline double fast_npow(double x, int p)
{
    double t = 1.0;
    for (int i = 0; i < -p; ++i)
    {
        t /= x;
    }
    return t;
}

inline uint32_t fast_ippow(uint32_t x, int p)
{
    uint32_t t = 1;
    for (int i = 0; i < p; ++i)
    {
        t *= x;
    }
    return t;
}

inline uint32_t fast_inpow(uint32_t x, int p)
{
    uint32_t t = 1;
    for (int i = 0; i < -p; ++i)
    {
        t /= x;
    }
    return t;
}

template <typename T>
T fast_ipow(T x, int p)
{
    if (p >= 0)
    {
        return fast_ippow(x, p);
    }
    return fast_inpow(x, p);
}

template <typename T>
T apply_exponent_scaler(T x, int e)
{
    if (e < 0)
    {
        for (int i = 0; i < -e; ++i)
        {
            x /= 10;
        }
    }
    else if (e > 0)
    {
        for (int i = 0; i < e; ++i)
        {
            x *= 10;
        }
    }
    return x;
}

#if INCLUDE_VDT
inline double fast_exp(const double x)
{
    return vdt::fast_exp(x);
}

inline double fast_log(const double x)
{
    return vdt::fast_log(x);
}

inline double fast_pow(const double x, const double y)
{
    return vdt::fast_exp(y * vdt::fast_log(x));
}

inline double fast_fabs(const double x)
{
    union di {
        double d;
        uint64_t i;
    };
    di t = {x};
    t.i &= std::numeric_limits<int64_t>::max();
    return t.d;
}

inline float fast_exp(const float x)
{
    return vdt::fast_expf(x);
}

inline float fast_log(const float x)
{
    return vdt::fast_logf(x);
}

inline float fast_pow(const float x, const float y)
{
    return vdt::fast_expf(y * vdt::fast_logf(x));
}

inline float fast_fabs(const float x)
{
    union di {
        float d;
        uint32_t i;
    };
    di t = {x};
    t.i &= std::numeric_limits<int32_t>::max();
    return t.d;
}

#else
inline double fast_exp(const double x)
{
    return exp(x);
}

inline double fast_log(const double x)
{
    return log(x);
}

inline double fast_pow(const double x, const double y)
{
    return pow(x, y);
}

inline double fast_fabs(const double x)
{
    return fabs(x);
}

inline float fast_exp(const float x)
{
    return expf(x);
}

inline float fast_log(const float x)
{
    return logf(x);
}

inline float fast_pow(const float x, const float y)
{
    return powf(x, y);
}

inline float fast_fabs(const float x)
{
    return fabsf(x);
}

#endif

inline float fast_sqrt(const float x)
{
    return ::sqrt(x);
}
inline double fast_sqrt(const double x)
{
    return ::sqrt(x);
}

inline uint64_t fast_abs(int64_t n)
{
    return n < 0 ? -n : n;
}

// fast approximate exp() function
union double_overlay {
    double d;
    struct // reverse for big endian
    {
        int j;
        int i;
    } n;
};
// y must be in range -700.0 .. 700.0
inline double approx_exp(double y)
{
    double_overlay eco;
    eco.d = 0.0;
    eco.n.i = (1512780.769564044 * y) + 1072632447.0;
    return eco.d;
}

} // namespace math
} // namespace miye
