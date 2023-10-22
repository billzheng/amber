/*
 * syscalls_files.hpp
 * Purpose: wrap file syscalls
 * Author:
 */

#pragma once
#include "libcore/utils/syscalls_returnchecks.hpp"
#include <fcntl.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace miye
{
namespace syscalls
{

template <bool Throws = false>
static inline int open(const char* pathname, int flags,
                       mode_t mode = 0) noexcept(!Throws)
{
    int retval = ::open(pathname, flags, mode);
    check_not_neg(retval, __func__, pathname);
    return retval;
}

template <bool Throws = false>
static inline int close(int fd) noexcept(!Throws)
{
    int retval = ::close(fd);
    check_not_neg(retval, __func__);
    return retval;
}

template <bool Throws = false>
static inline ssize_t read(int fd, void* buf, size_t count) noexcept(!Throws)
{
    ssize_t retval = ::read(fd, buf, count);
    check_not_neg(retval, __func__);
    return retval;
}

template <bool Throws = false>
static inline ssize_t write(int fd, const void* buf,
                            size_t count) noexcept(!Throws)
{
    ssize_t retval = ::write(fd, buf, count);
    check_not_neg(retval, __func__);
    return retval;
}

template <bool Throws = false>
static inline int ftruncate64(int fd, off64_t length) noexcept(!Throws)
{
    int retval = ::ftruncate64(fd, length);
    check_not_neg(retval, __func__);
    return retval;
}

template <bool Throws = false>
static inline int dup(int oldfd) noexcept(!Throws)
{
    int retval = ::dup(oldfd);
    check_not_neg(retval, __func__);
    return retval;
}

template <bool Throws = false>
static inline int fstat64(int fd, struct stat64* buf) noexcept(!Throws)
{
    int retval = ::fstat64(fd, buf);
    check_not_neg(retval, __func__);
    return retval;
}

template <bool Throws = false>
static inline int stat64(const char* path, struct stat64* buf) noexcept(!Throws)
{
    int retval = ::stat64(path, buf);
    check_not_neg(retval, __func__);
    return retval;
}

template <bool Throws = false>
static inline int flock(int fd, int operation) noexcept(!Throws)
{
    int retval = ::flock(fd, operation);
    check_not_neg(retval, __func__);
    return retval;
}

template <bool Throws = false>
static inline int unlink(const char* pathname) noexcept(!Throws)
{
    int retval = ::unlink(pathname);
    check_not_neg(retval, __func__);
    return retval;
}

} // namespace syscalls
} // namespace miye
