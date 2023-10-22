/*
 * array.hpp
 * Purpose wrap an STL std::array to detect index out of bounds
 */

#include "libcore/essential/assert.hpp"
#include <array>
#include <initializer_list>

#pragma once

namespace miye
{
namespace fundamentals
{

template <typename T, int N>
class array : public std::array<T, N>
{
    typedef array<T, N> array_t;
    typedef std::array<T, N> parent;

  public:
    array() = default;

    array(std::initializer_list<T> l) : parent(l)
    {
    }

    // asserts below disappear unless compiled in debug mode
    // in release mode whole thing collapses to exactly an stl array
    T& operator[](size_t idx)
    {
        ASSERT_MSG(idx < parent::size(),
                   "index=" << idx << " is not less than array size="
                            << parent::size());
        return parent::operator[](idx);
    }
    const T& operator[](size_t idx) const
    {
        ASSERT_MSG(idx < parent::size(),
                   "index=" << idx << " is not less than array size="
                            << parent::size());
        return parent::operator[](idx);
    }
};
} // namespace fundamentals
} // namespace miye

template <typename OS, typename T, int N>
inline OS& operator<<(OS& os, miye::fundamentals::array<T, N>& a)
{
    os << "[ ";
    for (size_t i = 0; i != a.size(); ++i)
    {
        os << a[i];
        if (i != (a.size() - 1))
        {
            os << ", ";
        }
    }
    os << " ]";
    return os;
}
