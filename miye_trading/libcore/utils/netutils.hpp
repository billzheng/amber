/*
 *
 * Purpose: network utilities
 *
 * Author: 
 */
#pragma once

#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <map>

#include "libcore/essential/assert.hpp"
#include "libcore/parsing/visit.hpp"

namespace miye
{
namespace utils
{

static const size_t MAX_SUBSCRIPTIONS_PER_SOCKET = 20;

enum class multicast_result_t : uint8_t
{
    ok = 0,
    device_fail = 1,
    socket_fail = 2,
    buffer_fail = 3
};

inline const char* c_str(multicast_result_t type)
{
    switch (type)
    {
    case multicast_result_t::ok:
        return "ok";
    case multicast_result_t::device_fail:
        return "device_fail";
    case multicast_result_t::socket_fail:
        return "socket_fail";
    case multicast_result_t::buffer_fail:
        return "buffer_fail";
    }
    return "unknown";
}

template <typename OStream>
OStream& operator<<(OStream& os, multicast_result_t m)
{
    os << c_str(m);
    return os;
}

inline std::string uint32_ip_to_string(uint32_t add)
{
    in_addr ip;
    ip.s_addr = add;
    std::ostringstream oss;
    oss << inet_ntoa(ip);
    return oss.str();
}

inline uint32_t str_to_ip_uint32_t(const std::string& address)
{
    in_addr ip;
    INVARIANT_MSG(inet_aton(address.c_str(), &ip) != 0,
                  "Invalid address: " << address << "\n");
    return ip.s_addr;
}

struct address_port_t
{
    address_port_t(std::string const& address, const uint32_t port) : port(port)
    {
        INVARIANT_MSG(inet_aton(address.c_str(), &ip) != 0,
                      "Invalid address: " << address << "\n");
    }

    address_port_t(const uint32_t address, const uint32_t port) : port(port)
    {
        ip.s_addr = address;
    }

    bool operator==(const address_port_t& rhs) const
    {
        return ip.s_addr == rhs.ip.s_addr && port == rhs.port;
    }
    bool operator<(const address_port_t& rhs) const
    {
        return (ip.s_addr < rhs.ip.s_addr) || (port < rhs.port);
    }

    in_addr ip;
    uint32_t port;
};

template <typename OStream>
OStream& operator<<(OStream& os, address_port_t m)
{
    os << inet_ntoa(m.ip) << ":" << m.port;
    return os;
}

struct multicast_address_t
{
    multicast_address_t()
    {
        ip_.s_addr = 0;
    }
    multicast_address_t(std::string const& address, std::string const& intf)
    {
        in_addr i;
        INVARIANT_MSG(inet_aton(address.c_str(), &i) != 0,
                      "Invalid address: " << address);
        ip_ = i;
        interface_ = intf;
    }

    const in_addr ip() const
    {
        return ip_;
    }

    std::string ip_as_str() const
    {
        return inet_ntoa(ip_);
    }

    const std::string interface() const
    {
        return interface_;
    }

    bool operator==(const multicast_address_t& rhs) const
    {
        return ip().s_addr == rhs.ip().s_addr && interface() == rhs.interface();
    }
    bool operator<(const multicast_address_t& rhs) const
    {
        return (ip().s_addr < rhs.ip().s_addr) ||
               (ip().s_addr == rhs.ip().s_addr &&
                interface() < rhs.interface());
    }

    std::string str() const
    {
        std::ostringstream oss;
        oss << inet_ntoa(ip_) << ":" << interface_;
        return oss.str();
    }

    VISITOR
    {
        VISIT_MANGLED("ip", ip_.s_addr);
        VISIT_MANGLED("interface", interface_);
    }

    in_addr ip_;
    std::string interface_;
};

template <typename OStream>
OStream& operator<<(OStream& os, multicast_address_t m)
{
    os << m.str().c_str();
    return os;
}

struct multicast_subscription_manager
{
    struct sub_fd_t
    {
        ip_mreqn mreq;
        int fd;
        uint32_t count;

        sub_fd_t() : fd(0), count(0)
        {
        }
        sub_fd_t(ip_mreqn mreq, int fd) : mreq(mreq), fd(fd), count(1)
        {
        }
    };

    multicast_result_t subscribe(multicast_address_t m)
    {
        if (addresses.count(m))
        {
            addresses[m].count += 1;
            return multicast_result_t::ok;
        }
        else
        {
            return do_subscription(m);
        }
    }

    multicast_result_t unsubscribe(multicast_address_t m)
    {
        if (addresses.count(m))
        {
            addresses[m].count -= 1;
            if (addresses[m].count == 0)
                return do_unsubscribe(m);
        }
        return multicast_result_t::ok;
    }

  private:
    int get_file_descriptor()
    {
        for (auto& f : fd_counts)
        {
            if (f.second < MAX_SUBSCRIPTIONS_PER_SOCKET)
                return f.first;
        }

        int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
        fd_counts[fd] = 0;
        return fd;
    }

    multicast_result_t do_subscription(multicast_address_t const& m)
    {
        int fd = get_file_descriptor();

        struct ifreq ifr;

        strncpy(ifr.ifr_name, m.interface().c_str(), IFNAMSIZ);
        if (ioctl(fd, SIOCGIFINDEX, &ifr) == -1)
        {
            // std::cerr << "calling ioctl(" << fd << ",
            // SIOCGIFINDEX="<<SIOCGIFINDEX << "&ifr" << ifr.ifr_name << ")\n";
            return multicast_result_t::device_fail;
        }

        struct ip_mreqn mreq;
        mreq.imr_multiaddr.s_addr = m.ip().s_addr;
        mreq.imr_address.s_addr = INADDR_ANY;
        mreq.imr_ifindex = ifr.ifr_ifindex;

        if (setsockopt(
                fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) == -1)
        {
            if (errno == ENOBUFS)
            {
                fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
                if (fd == -1)
                    return multicast_result_t::socket_fail;
            }
            else
            {
                return multicast_result_t::buffer_fail;
            }
        }
        fd_counts[fd] += 1;
        addresses.emplace(m, sub_fd_t(mreq, fd));
        return multicast_result_t::ok;
    }

    multicast_result_t do_unsubscribe(multicast_address_t const& m)
    {
        auto& sub = addresses[m];

        if (setsockopt(sub.fd,
                       IPPROTO_IP,
                       IP_DROP_MEMBERSHIP,
                       &sub.mreq,
                       sizeof(sub.mreq)) == -1)
            return multicast_result_t::socket_fail;
        fd_counts[sub.fd] -= 1;
        return multicast_result_t::ok;
    }

    std::map<multicast_address_t, sub_fd_t> addresses;
    std::map<int, uint32_t> fd_counts;
};

} // namespace utils
} // namespace miye

namespace std
{

template <>
struct hash<miye::utils::address_port_t>
{
  public:
    size_t operator()(const miye::utils::address_port_t& rhs) const
    {
        return (rhs.ip.s_addr << 4) + rhs.port;
    }
};

template <>
struct hash<miye::utils::multicast_address_t>
{
  public:
    size_t operator()(const miye::utils::multicast_address_t& rhs) const
    {
        return rhs.ip().s_addr;
    }
};
} // namespace std
