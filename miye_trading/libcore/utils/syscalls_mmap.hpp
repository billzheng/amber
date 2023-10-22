/*
 * syscalls_mmap.hpp
 * Purpose: wrap mmap systemcalls
 */

#pragma once

#include "libcore/utils/syscalls_returnchecks.hpp"
#include <sys/mman.h>

namespace miye
{
namespace syscalls
{

template <bool Throws = false>
static inline void* mmap64(void* addr, size_t length, int prot, int flags,
                           int fd, off64_t offset) noexcept(!Throws)
{
    void* retval = ::mmap(addr, length, prot, flags, fd, offset);
    check_notval(retval, MAP_FAILED, __func__);
    return retval;
}

template <bool Throws = false>
static inline int munmap(void* addr, size_t length) noexcept(!Throws)
{
    int retval = ::munmap(addr, length);
    check_not_neg(retval, __func__);
    return retval;
}

template <bool Throws = false>
static inline int msync(void* addr, size_t length, int flags) noexcept(!Throws)
{
    int retval = ::msync(addr, length, flags);
    check_not_neg(retval, __func__);
    return retval;
}

} // namespace syscalls
} // namespace miye
