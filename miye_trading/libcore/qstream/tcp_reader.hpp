/*
 * tcp_reader.hpp
 * Purpose: read from a connected tcp socket
 * Author: 
 */

#include "libcore/utils/syscalls_files.hpp"
#include "libcore/utils/syscalls_sockets.hpp"
#include "qstream_reader_interface.hpp"

#pragma once

namespace miye
{
namespace qstream
{

// How could we get a reader?
// Listening socket returned an fd with accept() given to constructor here
// writer performing an active connection, it's fd given to the reader.
// can't have a reader make a connection - can't give this an ip:port

template <typename Clock>
class tcp_reader : public qstream_reader_interface<tcp_reader<Clock>>
{
  public:
    tcp_reader() = delete;
    tcp_reader(std::string) = delete;
    tcp_reader(Clock& clock_, int fd, std::string descrip)
        : description(descrip), clock(clock_)
    {
        socket_fd = fd;
    }

    const std::string& describe() const
    {
        return description;
    }
    const place read()
    {
        int flags = 0;
        ssize_t bytes_read =
            ::recv(socket_fd, readbuffer, sizeof(readbuffer), flags);
        if (bytes_read <= 0)
        {
            return place::eof();
        }
        return place(readbuffer, bytes_read);
    }

    int get_fd()
    {
        return socket_fd;
    }
    void attest(uint64_t* next_timestamp)
    {
        UNUSED(next_timestamp);
    }

    void reset_fd(int new_fd, std::string new_ip)
    {
        socket_fd = new_fd;
        ip = new_ip;
        description += ",changed_to=" + new_ip;
    }

    ~tcp_reader()
    {
    }

    int socket_fd;
    char readbuffer[SUPERPAGE_SIZE];
    std::string ip;
    std::string description;
    Clock& clock;
};

} // namespace qstream
} // namespace miye
