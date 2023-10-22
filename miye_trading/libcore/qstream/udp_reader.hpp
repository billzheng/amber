/*
 * udp_reader.hpp
 * Purpose: read from a udp socket
 * Author: 
 */

#pragma once
#include "arbiter_common.hpp"
#include "ip_common.hpp"
#include "libcore/utils/syscalls_sockets.hpp"
#include "qstream_common.hpp"
#include "qstream_reader_interface.hpp"
#include <linux/net_tstamp.h>
#include <linux/sockios.h>
#include <vector>

namespace miye
{
namespace qstream
{

template <typename Clock>
class udp_reader : public qstream_reader_interface<udp_reader<Clock>>
{
  public:
    udp_reader() = delete;
    udp_reader(const udp_reader&) = delete;
    udp_reader(const udp_reader&&) = delete;

    udp_reader(Clock& clock_, std::string descrip)
        : qstream_reader_interface<udp_reader<Clock>>(), clock(clock_),
          description(descrip), is_subscribed(false)
    {
        auto options = extract_streamoptions(description);
        adapter_ts = is_adapter_ts(options);
        timed = is_timed(options) && clock.can_set();
        socket_fd = syscalls::socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
        int flag = 1;
        syscalls::setsockopt(
            socket_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(flag));
        syscalls::setsockopt(
            socket_fd, SOL_SOCKET, SO_REUSEPORT, (char*)&flag, sizeof(flag));

        // bump the buffer size to 15M
        int buffer_size = 15 * 1024 * 1024;
        syscalls::setsockopt(socket_fd,
                             SOL_SOCKET,
                             SO_RCVBUF,
                             &buffer_size,
                             sizeof(buffer_size));
        set_nonblocking(socket_fd);

        ifname = extract_ifname(description);
        ip = extract_ip(description);
        if (is_class_D(ip))
        {
            subscribe_mcast();
        }
        uint16_t port = extract_port(description);
        INVARIANT(port > 0);
        std::cout << "udp listen socket " << DUMP(socket_fd) << std::endl;
        std::cout << DUMP(ifname) << std::endl;
        std::cout << DUMP(ip) << std::endl;
        std::cout << DUMP(port) << std::endl;
        ::memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET;
        if (ip.compare("*") == 0)
        {
            sa.sin_addr.s_addr = htonl(INADDR_ANY);
        }
        else
        {
            inet_pton(AF_INET, ip.c_str(), &sa.sin_addr);
        }
        sa.sin_port = htons(port);
        syscalls::bind(socket_fd, (const struct sockaddr*)&sa, sizeof(sa));
        if (adapter_ts)
        {
            // envirnoment variable EF_RX_TIMESTAMPING=3 (1, if you want
            // fallback not fail)
            int val = SOF_TIMESTAMPING_RX_HARDWARE;
            val |= SOF_TIMESTAMPING_RAW_HARDWARE;
            syscalls::setsockopt(socket_fd,
                                 SOL_SOCKET,
                                 SO_TIMESTAMPING,
                                 (void*)&val,
                                 sizeof(val));
        }
    }

    const std::string& describe() const
    {
        return description;
    }
    int get_fd()
    {
        return socket_fd;
    }

    void change_subscription(bool connect)
    {
        if (socket_fd == -1)
        {
            return;
        }
        int optname = IP_ADD_MEMBERSHIP;
        if (!connect)
        {
            std::cerr << ip << " ...leaving "
                      << "\n";
            optname = IP_DROP_MEMBERSHIP;
        }
        else
        {
            std::cerr << ip << "...joining "
                      << "\n";
        }

        struct sockaddr_in s_gaddr;
        ::memset(&s_gaddr, 0, sizeof(s_gaddr));
        s_gaddr.sin_family = AF_INET;
        inet_pton(AF_INET, ip.c_str(), &s_gaddr.sin_addr);

        struct ip_mreqn req;
        ::memset(&req, 0, sizeof(req));
        req.imr_address.s_addr = INADDR_ANY;
        req.imr_multiaddr = s_gaddr.sin_addr;
        if (ifname.length())
        {
            req.imr_ifindex = get_ifindex(socket_fd, ifname);
        }
        // this should send an igmp join or leave msg
        syscalls::setsockopt(socket_fd, IPPROTO_IP, optname, &req, sizeof(req));
        is_subscribed = connect;
        std::cerr << "... done " << ip << "\n";
    }
    void subscribe_mcast()
    {
        if (!is_subscribed)
        {
            change_subscription(true);
        }
    }

    void unsubscribe_mcast()
    {
        if (is_subscribed)
        {
            change_subscription(false);
        }
    }

    void attest(uint64_t* next_timestamp)
    {
        UNUSED(next_timestamp);
    }

    const place read()
    {
        int bytes_read = 0;
        if (adapter_ts & timed)
        {
            memset(&msg, 0, sizeof(msg));
            msg.msg_iov = &entry;
            msg.msg_iovlen = 1;
            entry.iov_base = readbuffer;
            entry.iov_len = sizeof(readbuffer);
            msg.msg_name = (caddr_t)&from_addr;
            msg.msg_namelen = sizeof(from_addr);
            msg.msg_control = &control;
            msg.msg_controllen = sizeof(control);
            bytes_read = recvmsg(socket_fd, &msg, MSG_DONTWAIT);
            if (bytes_read < 0 && errno == EAGAIN)
            {
                bytes_read = 0;
            }
            cmsghdr* cmsg;
            uint64_t timestamp = 0;
            for (cmsg = CMSG_FIRSTHDR(&msg); cmsg;
                 cmsg = CMSG_NXTHDR(&msg, cmsg))
            {
                if (cmsg->cmsg_level == SOL_SOCKET)
                {
                    if (cmsg->cmsg_type == SO_TIMESTAMPING)
                    {
                        struct timespec* ts = (struct timespec*)CMSG_DATA(cmsg);
                        ts += 2;
                        timestamp = time::seconds(ts->tv_sec) +
                                    time::nanos(ts->tv_nsec);
                        clock.set(timestamp);
                        break;
                    }
                }
            }
        }
        else
        {
            int flags = 0;
            socklen_t len = 0;
            struct sockaddr from_sa;
            bytes_read = ::recvfrom(socket_fd,
                                    (void*)readbuffer,
                                    sizeof(readbuffer),
                                    flags,
                                    (struct sockaddr*)&from_sa,
                                    &len);
            if (bytes_read < 0 && errno == EAGAIN)
            {
                bytes_read = 0;
            }
        }
        return place(readbuffer, bytes_read);
    }

    ~udp_reader()
    {
        std::cerr << __PRETTY_FUNCTION__ << " closing " << DUMP(socket_fd)
                  << std::endl;
        ::close(socket_fd);
        socket_fd = -1;
    }

    int socket_fd;
    bool adapter_ts;
    bool timed;
    struct sockaddr_in sa;
    Clock& clock;
    char readbuffer[SUPERPAGE_SIZE];
    std::string description;
    std::string ifname;
    std::string ip;
    bool is_subscribed;
    struct msghdr msg;
    struct iovec entry;
    struct sockaddr_in from_addr;
    struct
    {
        char control[512];
        struct cmsghdr cm;
    } control;
};

} // namespace qstream
} // namespace miye
