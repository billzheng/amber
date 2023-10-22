/*
 * utils.hpp
 * Purpose: general purpose
 */

#pragma once

namespace miye { namespace essential {



template<typename T>
constexpr T static_max(T left, T right) noexcept { return left > right ? left : right; }


// takes a list of types, returns the size of the largest
template<typename... All> struct max_size;
template<typename T>
struct max_size<T>
{
    constexpr static size_t size() { return sizeof(T);}
};
template<typename T, typename ...Args>
struct max_size<T, Args...>
{
    constexpr static size_t size() {
        return sizeof(T) > max_size<Args...>::size() ? sizeof(T) : max_size<Args...>::size();
    }
};

}}
