/*
 * tcp_listener.hpp
 * purpose: listening tcp socket
 * Author: 
 */

#pragma once

#include "ip_common.hpp"
#include "libcore/time/clock.hpp"
#include "libcore/time/time.hpp"
#include "libcore/utils/syscalls_files.hpp"
#include "libcore/utils/syscalls_sockets.hpp"
#include "qstream_common.hpp"
#include "qstream_reader_interface.hpp"
#include <iostream>
#include <string>

namespace miye
{
namespace qstream
{

struct new_connection
{
    int new_fd;
    std::string ip;
    uint16_t port;
    std::string description()
    {
        return std::string("tcp_l:" + ip + ":" + std::to_string(port));
    }
};

template <typename Clock>
class tcp_listener : public qstream_reader_interface<tcp_listener<Clock>>
{
  public:
    tcp_listener(Clock& clock_, std::string descrip)
        : clock(clock_), description(descrip)
    {
        listening_fd = syscalls::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        // set sockopts
        // switch off nagle
        int flag = 1;
        syscalls::setsockopt(
            listening_fd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag));
        // reuse address
        flag = 1;
        syscalls::setsockopt(
            listening_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, sizeof(flag));

        std::cerr << "created listening socket " << listening_fd << std::endl;
        struct sockaddr_in sa;
        ::memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET;
        // if nothing specified use any
        std::string ip = extract_ip(description);
        if (ip.compare("*") == 0)
        {
            sa.sin_addr.s_addr = htonl(INADDR_ANY);
        }
        else
        {
            inet_pton(AF_INET, ip.c_str(), &sa.sin_addr);
        }
        uint16_t port = extract_port(description);
        sa.sin_port = htons(port);
        syscalls::bind(listening_fd, (struct sockaddr*)&sa, sizeof(sa));
        int backlog_size = 15;
        syscalls::listen(listening_fd, backlog_size);
    }

    const std::string& describe() const
    {
        return description;
    }

    const place read()
    {
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        socklen_t len = sizeof(addr);
        last_connection.new_fd =
            ::accept(listening_fd, (struct sockaddr*)&addr, &len);
        char buf[BUFSIZ];
        inet_ntop(AF_INET, &addr.sin_addr, buf, BUFSIZ);
        last_connection.port = ntohs(addr.sin_port);
        last_connection.ip = std::string(buf);
        return place((char*)&last_connection, sizeof(last_connection));
    }
    int get_fd()
    {
        return listening_fd;
    }
    uint64_t attest(uint64_t* next_timestamp)
    {
        return time::max;
    }

    ~tcp_listener()
    {
        std::cerr << __PRETTY_FUNCTION__ << " closing listening fd "
                  << listening_fd << std::endl;
        syscalls::close(listening_fd);
    }

    int listening_fd;
    struct new_connection last_connection;
    Clock& clock;
    std::string description;
};

} // namespace qstream
} // namespace miye
