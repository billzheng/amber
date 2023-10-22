/*
 * syscalls_misc.hpp
 * Purpose: system call wrapping
 * Author: 
 */

#pragma once

#include "libcore/utils/syscalls_returnchecks.hpp"
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/inotify.h>

namespace miye
{
namespace syscalls
{

template <bool Throws = false>
static inline int poll(struct pollfd* fds, nfds_t nfds,
                       int timeout) noexcept(!Throws)
{
    int retval = ::poll(fds, nfds, timeout);
    if (retval < 0 && errno != EINTR)
    {
        check_not_neg(retval, __func__);
        return retval;
    }
    return retval;
}

template <bool Throws = false>
static inline int epoll_create()
{
    // size arg of ::epoll_create is ignored
    int retval = ::epoll_create(5);
    check_not_neg(retval, __func__);
    return retval;
}

template <bool Throws = false>
static inline int epoll_ctl(int epfd, int op, int fd, struct epoll_event* ev)
{
    int retval = ::epoll_ctl(epfd, op, fd, ev);
    check_not_neg(retval, __func__);
    return retval;
}

// signal stuff
template <bool Throws = false>
static inline int raise(int sig) noexcept(!Throws)
{
    int retval = ::raise(sig);
    check_not_neg(retval, __func__);
    return retval;
}

template <bool Throws = false>
static inline int sigaction(int signum, const struct sigaction* act,
                            struct sigaction* oldact) noexcept(!Throws)
{
    int retval = ::sigaction(signum, act, oldact);
    check_not_neg(retval, __func__);
    return retval;
}

// inotify
template <bool Throws = false>
static inline int inotify_init(int flags = 0) noexcept(!Throws)
{
    int fd = ::inotify_init();
    check_not_neg(fd, __func__);
    int retval = ::fcntl(fd, F_SETFL, flags);
    check_not_neg(retval, __func__);
    return fd;
}

template <bool Throws = false>
static inline int inotify_add_watch(int fd, const char* pathname,
                                    uint32_t mask) noexcept(!Throws)
{
    int retval = ::inotify_add_watch(fd, pathname, mask);
    check_not_neg(retval, __func__, pathname);
    return retval;
}

// other
template <bool Throws = false>
static inline int clock_gettime(clockid_t clk_id,
                                struct timespec* tp) noexcept(!Throws)
{
    int retval = ::clock_gettime(clk_id, tp);
    check_not_neg(retval, __func__);
    return retval;
}

template <bool Throws = false>
static inline char* getcwd(char* buf, size_t size) noexcept(!Throws)
{
    char* retval = ::getcwd(buf, size);
    check_nonnull(retval, __func__);
    return retval;
}
template <bool Throws = false>
static inline int posix_memalign(void** memptr, size_t alignment,
                                 size_t size) noexcept(!Throws)
{
    auto retval = ::posix_memalign(memptr, alignment, size);
    check_not_neg(retval, __func__);
    return retval;
}

} // namespace syscalls
} // namespace miye
