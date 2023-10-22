/*
 * sycalls_libc.hpp
 * Purpose wrap libc interface to syscalls
 * Author: 
 */
#pragma once
#include "libcore/utils/syscalls_returnchecks.hpp"

namespace miye
{
namespace syscalls
{

template <bool Throws = false>
static inline FILE* fopen(const char* path, const char* mode) noexcept(!Throws)
{
    FILE* fp = ::fopen(path, mode);
    check_nonnull(fp, __func__);
    return fp;
}

template <bool Throws = false>
static inline int fclose(FILE* fp) noexcept(!Throws)
{
    int retval = ::fclose(fp);
    check_not_eof(retval, __func__);
    return retval;
}

template <bool Throws = false>
static inline int fileno(FILE* stream) noexcept(!Throws)
{
    int retval = ::fileno(stream);
    check_not_neg(retval, __func__);
    return retval;
}

template <bool Throws = false>
static inline void* memset(void* s, int c, size_t n)
{
    void* retval = ::memset(s, c, n);
    check_nonnull(retval, __func__);
    return retval;
}

} // namespace syscalls
} // namespace miye
