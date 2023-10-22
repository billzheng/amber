/*
 * udp_writer.hpp
 * Purpose: write to a udp socket
 * Author: 
 */

#pragma once
#include "arbiter_common.hpp"
#include "ip_common.hpp"
#include "libcore/utils/syscalls_sockets.hpp"
#include "qstream_writer_interface.hpp"
#include <vector>

namespace miye
{
namespace qstream
{

template <typename Clock>
class udp_writer : public qstream_writer_interface<udp_writer<Clock>>
{
  public:
    udp_writer() = delete;
    udp_writer(Clock& clock_, std::string descrip)
        : qstream_writer_interface<udp_writer<Clock>>(), clock(clock_),
          description(descrip), is_multicast(false)
    {
        int fd = syscalls::socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
        int flag = 1;
        syscalls::setsockopt(
            fd, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(flag));

        std::string ip = extract_ip(description);
        is_multicast = is_class_D(ip);
        uint16_t port = extract_port(description);
        ifname = extract_ifname(description);
        std::cout << "udp write socket " << DUMP(fd) << std::endl;
        std::cout << DUMP(ifname) << std::endl;
        std::cout << DUMP(ip) << std::endl;
        std::cout << DUMP(port) << std::endl;
        struct sockaddr_in sa;
        ::memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET;
        sa.sin_port = htons(port);
        inet_pton(AF_INET, ip.c_str(), &sa.sin_addr);
        syscalls::connect(fd, (struct sockaddr*)&sa, sizeof(sa));
        if (is_multicast)
        {
            const int val = 1;
            syscalls::setsockopt(
                fd, IPPROTO_IP, IP_MULTICAST_LOOP, &val, sizeof(val));
            const u_char ttl = 1;
            syscalls::setsockopt(
                fd, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl));
        }

        socket_fd = fd;
    }

    const std::string& describe() const
    {
        return description;
    }
    place pledge(size_t n)
    {
        return place(writebuffer, n);
    }
    int announce(const place& p)
    {
        // copy writebuffer into the socket
        size_t written = 0;
        do
        {
            // note: don't want a sigpipe if the other end drops the connection
            ssize_t this_write = ::send(socket_fd,
                                        &writebuffer[written],
                                        p.size - written,
                                        MSG_NOSIGNAL);
            if (this_write < 0)
            {
                int err = errno;
                std::ostringstream os;
                os << "send_failed " << DUMP(socket_fd) << DUMP(written)
                   << std::endl;
                return err;
            }
            else
            {
                written += this_write;
            }
        } while (written != p.size);
        return 0; // success
    }
    int get_fd()
    {
        return socket_fd;
    }

    ~udp_writer()
    {
        std::cerr << __func__ << "() closing " << DUMP(socket_fd) << std::endl;
        ::close(socket_fd);
    }
    int socket_fd;
    std::vector<struct in_addr> subscribed_addresses;
    Clock& clock;
    char writebuffer[SUPERPAGE_SIZE];
    std::string description;
    std::string ifname;
    bool is_multicast;
};

} // namespace qstream
} // namespace miye
