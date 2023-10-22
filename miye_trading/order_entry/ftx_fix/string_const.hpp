#pragma once
#include <cstdint>
#include <cstring>
#include <cassert>

namespace acutus
{
/*
 * very naive version of strng_const.
 *
 */
class string_const
{
public:

    template <size_t N>
    constexpr explicit string_const(const char(&arr)[N]) noexcept
    : ptr_(arr[0]),
      len_(N - 1)
    {
    }

    constexpr explicit string_const(const char* p) noexcept
    : ptr_(p),
      len_(strLen(p))
    {
    }
    constexpr const char* begin() const noexcept { return ptr_; }  
    constexpr size_t size() const noexcept { return len_; }
    constexpr char operator[](std::size_t n) const noexcept 
    {
    	assert(n < len_ );
        return ptr_[n];

        // exception is disable in acutus
        //throw std::out_of_range("");
    }

    constexpr static inline size_t strLen(const char* p)
    {
        size_t i {};
        while (p[i] != '\0') ++i;
        return i;
    }
 
private:
    const char* const ptr_ {};
    const size_t len_ {};
};

} //namespace acutus
