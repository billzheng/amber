/*
 * tcp_common.cpp
 * Purpose: common routines for tcp_{listener,reader,writer}
 * Author: 
 */

#include "ip_common.hpp"
#include "libcore/essential/assert.hpp"
#include "libcore/utils/syscalls_sockets.hpp"
#include "qstream_common.hpp"
#include <fcntl.h>
#include <linux/sockios.h>
#include <net/if.h>
#include <sys/ioctl.h>

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <linux/if_link.h>
#include <netdb.h>

namespace miye
{
namespace qstream
{

std::string extract_ip_port(std::string description)
{
    auto start = description.find(":");
    auto end = description.find("@");
    return description.substr(start + 1, end - 1);
}
std::string extract_ip_str(std::string ip_port)
{
    auto port_start = ip_port.find(":");
    return ip_port.substr(0, port_start);
}
std::string extract_port_str(std::string ip_port)
{
    auto port_start = ip_port.find(":");
    if (port_start == std::string::npos)
    {
        return "0";
    }
    return ip_port.substr(port_start + 1, std::string::npos);
}

uint16_t extract_port(std::string description)
{
    std::string ip_port = extract_ip_port(description);
    std::string port_str = extract_port_str(ip_port);
    return std::stoi(port_str);
}
std::string extract_ip(std::string description)
{
    std::string ip_port = extract_ip_port(description);
    return extract_ip_str(ip_port);
}
std::string extract_ifname(std::string description)
{
    std::string key("ifname=");
    std::string ifname = extract_val_for_key(description, key);
    return ifname;
}

bool connect_can_fail(std::string description)
{
    return description.find("connectCanFail") != std::string::npos;
}

int get_ifindex(const int fd, std::string ifname)
{
    struct ifreq req;
    strncpy(req.ifr_name, ifname.c_str(), IFNAMSIZ);
    if (::ioctl(fd, SIOCGIFINDEX, &req) < 0)
    {
        INVARIANT_FAIL("ioctl error " << strerror(errno));
    }
    return req.ifr_ifindex;
}

bool is_class_D(std::string ip)
{
    // class D multiast addresses are in the range
    // 224.x.x.x to 239.x.x.x
    auto first_dot = ip.find(".");
    if (first_dot == std::string::npos)
    {
        return false;
    }
    auto first_octet = ip.substr(0, first_dot);
    auto i = ::atoi(first_octet.c_str());
    if (i >= 224 && i <= 239)
    {
        return true;
    }
    return false;
}

std::string ip_of_interface(std::string ifname)
{
    struct ifaddrs* ifaddr;
    struct ifaddrs* ifa;
    int s;
    char host[NI_MAXHOST];
    std::string local_ip_addr;

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    /* Walk through linked list, maintaining head pointer so we
       can free list later */

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
        {
            continue;
        }
        /* For an AF_INET interface address, get the address */
        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            int ret = strncmp(ifa->ifa_name, ifname.c_str(), ifname.size());
            if (ret == 0)
            {
                s = getnameinfo(ifa->ifa_addr,
                                sizeof(struct sockaddr_in),
                                host,
                                NI_MAXHOST,
                                NULL,
                                0,
                                NI_NUMERICHOST);
                if (s != 0)
                {
                    printf("getnameinfo() failed: %s\n", gai_strerror(s));
                    exit(EXIT_FAILURE);
                }
                printf("%s : %s\n", ifa->ifa_name, host);
                local_ip_addr = std::string(host);
            }
        }
    }

    freeifaddrs(ifaddr);
    return local_ip_addr;
}

void set_nonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    INVARIANT_MSG(flags >= 0, "fcntl failed, returned: " << flags);
    flags |= O_NONBLOCK;
    flags = fcntl(fd, F_SETFL, flags);
    INVARIANT_MSG(flags >= 0, "fcntl failed, returned: " << flags);
}

} // namespace qstream
} // namespace miye
