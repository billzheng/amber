/*
 * deque.hpp
 * Purpose wrap an STL deque to detect index out of bounds
 */

#include "libcore/essential/assert.hpp"
#include <deque>
#include <initializer_list>

#pragma once

namespace miye
{
namespace fundamentals
{

template <typename T>
class deque : public std::deque<T>
{
  public:
    explicit deque(size_t n = 0) : std::deque<T>(n)
    {
    }

    deque(size_t n, const T& val) : std::deque<T>(n, val)
    {
    }

    template <typename InIter>
    deque(InIter first, InIter last) : std::deque<T>(first, last)
    {
    }

    deque(std::initializer_list<T> l) : std::deque<T>(l)
    {
    }

    // asserts below disappear unless compiled in debug mode
    // in release mode whole thing collapses to exactly an stl deque
    T& operator[](size_t idx)
    {
        ASSERT_MSG(idx < std::deque<T>::size(),
                   "index=" << idx << " is not less than deque size="
                            << std::deque<T>::size());
        return std::deque<T>::operator[](idx);
    }
    const T& operator[](size_t idx) const
    {
        ASSERT_MSG(idx < std::deque<T>::size(),
                   "index=" << idx << " is not less than vector size="
                            << std::deque<T>::size());
        return std::deque<T>::operator[](idx);
    }
};

template <typename OS, typename T>
inline OS& operator<<(OS& os, const miye::fundamentals::deque<T>& d)
{
    os << "[\n";
    for (size_t i = 0; i != d.size(); ++i)
    {
        os << "[" << i << "] = " << d[i] << "\n";
    }
    os << "]";
    return os;
}

} // namespace fundamentals
} // namespace miye
