/*
 * vector.hpp
 * Purpose wrap an STL vector to detect index out of bounds
 */

#include "libcore/essential/assert.hpp"
#include <initializer_list>
#include <vector>

#pragma once

namespace miye
{
namespace fundamentals
{

template <typename T>
class vector : public std::vector<T>
{
  public:
    explicit vector(size_t n = 0) : std::vector<T>(n)
    {
    }

    vector(size_t n, const T& val) : std::vector<T>(n, val)
    {
    }

    template <typename InIter>
    vector(InIter first, InIter last) : std::vector<T>(first, last)
    {
    }

    vector(std::initializer_list<T> l) : std::vector<T>(l)
    {
    }

    // asserts below disappear unless compiled in debug mode
    // in release mode whole thing collapses to exactly an stl vector
    T& operator[](size_t idx)
    {
        ASSERT_MSG(idx < std::vector<T>::size(),
                   "index=" << idx << " is not less than vector size="
                            << std::vector<T>::size());
        return std::vector<T>::operator[](idx);
    }
    const T& operator[](size_t idx) const
    {
        ASSERT_MSG(idx < std::vector<T>::size(),
                   "index=" << idx << " is not less than vector size="
                            << std::vector<T>::size());
        return std::vector<T>::operator[](idx);
    }
};
} // namespace fundamentals
} // namespace miye

template <typename OS, typename T>
inline OS& operator<<(OS& os, miye::fundamentals::vector<T>& v)
{
    os << "[ ";
    for (size_t i = 0; i != v.size(); ++i)
    {
        os << v[i];
        if (i != (v.size() - 1))
        {
            os << ", ";
        }
    }
    os << " ]";
    return os;
}
