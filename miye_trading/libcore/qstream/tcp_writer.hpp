/*
 * tcp_writer.hpp
 * Purpose: read a connected tcp socket
 * Author: 
 */

#pragma once

#include "ip_common.hpp"
#include "libcore/time/timeutils.hpp"
#include "libcore/utils/syscalls_files.hpp"
#include "libcore/utils/syscalls_sockets.hpp"
#include "qstream_common.hpp"
#include "qstream_writer_interface.hpp"

// ensure this compiles on machines that don't
// have onload installed.
#include <linux/net_tstamp.h>
#include <time.h>
#define ONLOAD_SOF_TIMESTAMPING_STREAM (1 << 23)
struct onload_scm_timestamping_stream
{
    struct timespec first_sent;
    struct timespec last_sent;
    size_t len;
};

namespace miye
{
namespace qstream
{

template <typename Clock>
class tcp_writer : public qstream_writer_interface<tcp_writer<Clock>>
{
  public:
    tcp_writer(Clock& clock_, std::string descrip, bool can_fail = false)
        : connected_fd(-1), adapter_ts(false), clock(clock_),
          description(descrip)
    {
        auto options = extract_streamoptions(description);
        adapter_ts = is_adapter_ts(options);
        dest_port = extract_port(description);
        INVARIANT_MSG(dest_port > 0, DUMP(dest_port));
        dest_ip = extract_ip(description);
        can_fail |= connect_can_fail(description);

        ifname = extract_ifname(description);
        std::cerr << DUMP(description) << std::endl;
        reconnect(can_fail);
    }

    /*
     * for servers, when you have a tcp_reader and a
     * valid fd returned by accept()
     * create a writer so you can respond to the connection
     */
    tcp_writer(Clock& clock_, int fd, std::string descrip)
        : clock(clock_), description(descrip), dest_port(0)
    {
        connected_fd = fd;
    }

    void readdress(std::string new_dest_ip, uint16_t new_dest_port)
    {
        dest_ip = new_dest_ip;
        dest_port = new_dest_port;
        description =
            "changed_to=" + new_dest_ip + ":" + std::to_string(new_dest_port);
    }

    int reconnect(bool can_fail)
    {
        std::cerr << __func__ << "()\n";
        int fd = syscalls::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        std::cerr << DUMP(fd) << std::endl;

        if (false && adapter_ts)
        {
            int enable =
                SOF_TIMESTAMPING_TX_HARDWARE | SOF_TIMESTAMPING_SYS_HARDWARE |
                SOF_TIMESTAMPING_RAW_HARDWARE | ONLOAD_SOF_TIMESTAMPING_STREAM
                //| SOF_TIMESTAMPING_OPT_TSONLY
                //| SOF_TIMESTAMPING_OPT_CMSG
                //| SOF_TIMESTAMPING_OPT_ID
                ;
            std::cerr << "setting sockopt for adapter timstamps " << std::hex
                      << "0x" << enable << std::dec << "\n";
            syscalls::setsockopt(
                fd, SOL_SOCKET, SO_TIMESTAMPING, &enable, sizeof(enable));
        }

        // switch of nagle
        int flag = 1;
        syscalls::setsockopt(
            fd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
        std::cerr << "nagle done" << std::endl;

        if (ifname.length())
        {
            // bind the socket to a local interface if the ifname=eth$whatever
            // is specified
            std::string local_ip_addr = ip_of_interface(ifname);
            std::cerr << DUMP(ifname) << DUMP(local_ip_addr) << std::endl;
            struct sockaddr_in sa_local;
            ::memset(&sa_local, 0, sizeof(sa_local));
            sa_local.sin_family = AF_INET;
            inet_pton(AF_INET, local_ip_addr.c_str(), &sa_local.sin_addr);

#if 1
            int ret = ::bind(fd, (struct sockaddr*)&sa_local, sizeof(sa_local));
#else
            // BINDTODEVICE fails. "Operation not permitted" have to be root.
            int ret = ::setsockopt(
                fd, SOL_SOCKET, SO_BINDTODEVICE, ifname.c_str(), ifname.size());
#endif
            if (ret)
            {
                perror("bind failed");
                std::cerr << DUMP(ret) << DUMP(ifname) << std::endl;
            }
            else
            {
                std::cerr << "bound to " << DUMP(ifname) << std::endl;
            }
        }

        struct sockaddr_in sa;
        ::memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET;
        sa.sin_port = htons(dest_port);
        inet_pton(AF_INET, dest_ip.c_str(), &sa.sin_addr);
        if (can_fail)
        {
            auto ret = ::connect(fd, (struct sockaddr*)&sa, sizeof(sa));
            auto saved_errno = errno;
            if (ret < 0)
            {
                std::cerr << __PRETTY_FUNCTION__ << " failed to connect to "
                          << dest_ip << std::endl;
                ::close(fd);
                connected_fd = -1;
                return saved_errno;
            }
        }
        else
        {
            std::cerr << "connecting...\n";
            syscalls::connect(fd, (struct sockaddr*)&sa, sizeof(sa));
            std::cerr << "connected...\n";
        }

        connected_fd = fd;
        std::cerr << __PRETTY_FUNCTION__ << " " << DUMP(connected_fd)
                  << std::endl;
        return 0;
    }

    int disconnect()
    {
        std::cerr << "disconnecting " << DUMP(connected_fd) << "\n";
        if (connected_fd > 0)
        {
            int ret = ::close(connected_fd);
            if (ret < 0)
            {
                return errno;
            }
        }
        connected_fd = -1;
        return 0;
    }

    bool connection_succeeded()
    {
        return get_fd() >= 0;
    }
    int get_fd()
    {
        return connected_fd;
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
            ssize_t this_write = ::send(connected_fd,
                                        &writebuffer[written],
                                        p.size - written,
                                        MSG_NOSIGNAL);
            if (this_write < 0)
            {
                int err = errno;
                std::ostringstream os;
                os << "\tsend_failed " << DUMP(connected_fd) << DUMP(written)
                   << std::endl;
                ::perror(os.str().c_str());
                return err;
            }
            else
            {
                written += this_write;
            }
        } while (written != p.size);
        return 0; // success
    }

    uint64_t current_system_ts()
    {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        return (time::seconds(ts.tv_sec) + time::nanos(ts.tv_nsec));
    }

    uint64_t last_send_ts()
    {
        // not yet working, just get the system time
        if (true || !adapter_ts)
        {
            return current_system_ts();
        }
        int checks = 0;
        const int max_checks = 999999;
        struct msghdr msg;
        struct iovec iov;
        struct sockaddr_in host_addr;
        ::memset(&host_addr, 0, sizeof(host_addr));
        host_addr.sin_family = AF_INET;
        host_addr.sin_port = 0;
        host_addr.sin_addr.s_addr = INADDR_ANY;

        iov.iov_base = recvmsg_buffer;
        iov.iov_len = sizeof(recvmsg_buffer);
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_name = &host_addr;
        msg.msg_namelen = sizeof(host_addr);
        msg.msg_control = control;
        msg.msg_controllen = sizeof(control);
        auto ts = current_system_ts();
        int ret = 0;

        do
        {
            msg.msg_controllen = 1024;
            ret = recvmsg(connected_fd, &msg, MSG_ERRQUEUE);
        } while ((ret < 0) && (errno == EAGAIN) && (++checks < max_checks));
        if (ret < 0)
        {
            if (errno == EAGAIN)
            {
                // gave up
                ASSERT(checks == max_checks);
                std::cerr << "\n\tgave up trying to get the tx timestamp\n\n";
                return ts;
            }
            else
            {
                // fatal error?
                int saved_errno = errno;
                INVARIANT_FAIL("error reading error queue" << DUMP(ret)
                                                           << DUMP(saved_errno)
                                                           << DUMP(checks));
            }
        }
        // should have the timestamp in the message here
        struct cmsghdr* cmsg;
        struct msghdr* msg_ptr = &msg;
        struct onload_scm_timestamping_stream* tcp_tx_stamps;
        for (cmsg = CMSG_FIRSTHDR(msg_ptr); cmsg != NULL;
             cmsg = CMSG_NXTHDR(msg_ptr, cmsg))
        {
            if (cmsg->cmsg_level != SOL_SOCKET)
            {
                continue;
            }
            switch (cmsg->cmsg_type)
            {
            case ONLOAD_SOF_TIMESTAMPING_STREAM:
                tcp_tx_stamps =
                    (struct onload_scm_timestamping_stream*)CMSG_DATA(cmsg);
                // conservative value for latency measure is last sent
                // tcp_tx_stamps->first_sent;
                // tcp_tx_stamps->len;
                // can also get sending time and rate (last - first)/len or
                // whatever if desired last_sent is always zero when running the
                // SF test program tx_timestamps.c so we have to used first_sent
                return (time::seconds(tcp_tx_stamps->first_sent.tv_sec) +
                        time::nanos(tcp_tx_stamps->first_sent.tv_nsec));
            default:
                /* Ignore other cmsg options */
                break;
            }
        }
        // shouldn't get here
        // but if we do the current system time is the most conservative
        // value we can report, we must have sent before now
        std::cerr << "failed to get the tx timestamp for reasons unknown\n";
        return ts;
    }

    ~tcp_writer()
    {
        std::cerr << __PRETTY_FUNCTION__ << " closing " << DUMP(connected_fd)
                  << std::endl;
        auto ret = ::close(connected_fd);
        if (ret < 0)
        {
            // already closed by the arbiter
        }
    }

    int connected_fd;
    bool adapter_ts;
    Clock& clock;
    char writebuffer[SUPERPAGE_SIZE];
    char recvmsg_buffer[2048];
    char control[1024];
    std::string description;
    uint16_t dest_port;
    std::string dest_ip;
    std::string ifname;
};

} // namespace qstream
} // namespace miye
