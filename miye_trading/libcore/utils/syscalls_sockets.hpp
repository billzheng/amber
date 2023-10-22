/*
 * syscalls_sockets.hpp
 * Purpose: wrap networking syscalls
 * Author: 
 */

#pragma once

#include "libcore/utils/syscalls_returnchecks.hpp"
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <sys/socket.h>

namespace miye
{
namespace syscalls
{

template <bool Throws = false>
static inline int socket(int domain, int type, int protocol) noexcept(!Throws)
{
    int retval = ::socket(domain, type, protocol);
    check_not_neg(retval, __func__);
    return retval;
}

template <bool Throws = false>
static inline int bind(int sockfd, const struct sockaddr* addr,
                       socklen_t addrlen) noexcept(!Throws)
{
    int retval = ::bind(sockfd, addr, addrlen);
    check_not_neg(retval, __func__);
    return retval;
}

template <bool Throws = false>
static inline int listen(int sockfd, int backlog) noexcept(!Throws)
{
    int retval = ::listen(sockfd, backlog);
    check_not_neg(retval, __func__);
    return retval;
}

template <bool Throws = false>
static inline int accept(int sockfd, struct sockaddr* addr,
                         socklen_t* addrlen) noexcept(!Throws)
{
    int retval = ::accept(sockfd, addr, addrlen);
    check_not_neg(retval, __func__);
    return retval;
}

template <bool Throws = false>
static inline ssize_t recv(int sockfd, void* buf, size_t len,
                           int flags) noexcept(!Throws)
{
    ssize_t retval = ::recv(sockfd, buf, len, flags);
    check_not_neg(retval, __func__);
    return retval;
}

template <bool Throws = false>
static inline ssize_t recvmsg(int sockfd, struct msghdr* msg,
                              int flags) noexcept(!Throws)
{
    ssize_t retval = ::recvmsg(sockfd, msg, flags);
    check_not_neg(retval, __func__);
    return retval;
}

template <bool Throws = false>
static inline ssize_t recvfrom(int sockfd, void* buf, size_t len, int flags,
                               struct sockaddr* src_addr,
                               socklen_t* addrlen) noexcept(!Throws)
{
    ssize_t retval = ::recvfrom(sockfd, buf, len, flags, src_addr, addrlen);
    check_not_neg(retval, __func__);
    return retval;
}

template <bool Throws = false>
static inline ssize_t send(int sockfd, const void* buf, size_t len,
                           int flags) noexcept(!Throws)
{
    ssize_t retval = ::send(sockfd, buf, len, flags);
    check_not_neg(retval, __func__);
    return retval;
}

template <bool Throws = false>
static inline ssize_t sendto(int sockfd, const void* buf, size_t len, int flags,
                             const struct sockaddr* dest_addr,
                             socklen_t addrlen) noexcept(!Throws)
{
    ssize_t retval = ::sendto(sockfd, buf, len, flags, dest_addr, addrlen);
    check_not_neg(retval, __func__);
    return retval;
}

template <bool Throws = false>
static inline int connect(int sockfd, const struct sockaddr* addr,
                          socklen_t addrlen) noexcept(!Throws)
{
    int retval = ::connect(sockfd, addr, addrlen);
    if (retval < 0 && errno != EINTR)
    {
        check_not_neg(retval, __func__);
    }
    // returns -1 if we were interrupted rather than throwing
    return retval;
}

template <bool Throws = false>
static inline int setsockopt(int sockfd, int level, int optname,
                             const void* optval,
                             socklen_t optlen) noexcept(!Throws)
{
    int retval = ::setsockopt(sockfd, level, optname, optval, optlen);
    check_not_neg(retval, __func__);
    return retval;
}

template <bool Throws = false>
static inline int shutdown(int sockfd, int how) noexcept(!Throws)
{
    int retval = ::shutdown(sockfd, how);
    check_not_neg(retval, __func__);
    return retval;
}

} // namespace syscalls
} // namespace miye
