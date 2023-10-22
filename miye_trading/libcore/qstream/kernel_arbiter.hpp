/*
 * kernel arbiter.hpp
 * Purpose: arbitrate between streams in favour of lowest unread timestamp
 * where streams require a syscall
 * Author: 
 */

#pragma once

#include "arbiter_common.hpp"
#include "ip_common.hpp"
#include "libcore/essential/assert.hpp"
#include "libcore/essential/platform_defs.hpp"
#include "libcore/essential/utils.hpp"
#include "libcore/time/clock.hpp"
#include "libcore/utils/array.hpp"
#include "libcore/utils/vector.hpp"

namespace miye
{
namespace qstream
{

template <typename Clock>
class kernel_arbiter
{
  public:
    kernel_arbiter(Clock& clk_)
        : clock(clk_), num_ready(0), streams_under_arbitration(0)
    {
        next_timestamp.reserve(arbiter_error::end);
        epfd = syscalls::epoll_create();
        ::memset((void*)&ev_list[0], 0, sizeof(ev_list));
    }

    stream_id_t submit(int fd, stream_id_t hint = 0) noexcept
    {
        // submit a stream for arbitration by the arbitrater
        // returns the index it will use to id
        // the stream when it reports arbitration
        stream_id_t this_stream_index = hint;
        for (; this_stream_index != fds.size(); ++this_stream_index)
        {
            if (fds[this_stream_index] == freed_id)
            {
                // slot avaialable for reuse
                // update with the timsetamp of its next read
                fds[this_stream_index] = fd;
                next_timestamp[this_stream_index] = time::max;
                break;
            }
        }
        if (this_stream_index == fds.size())
        {
            // update with the timsetamp of its next read
            next_timestamp.push_back(time::max);
            fds.push_back(fd);
            INVARIANT(fds.size() == next_timestamp.size());
        }

        // needed for user-level epoll
        set_nonblocking(fd);

        struct epoll_event ev;
        ::memset(&ev, 0, sizeof(ev));
        ev.data.fd = fd;
        ev.events = EPOLLIN; //|EPOLLET;
        syscalls::epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);

        INVARIANT(next_timestamp.size() == fds.size());
        INVARIANT(next_timestamp.size() < arbiter_error::end);

        ++streams_under_arbitration;
        return this_stream_index;
    }

    void withdraw(stream_id_t id) noexcept
    {
        // withdraws the stream with id from arbitration
        // e.g. the other end terminates the connection
        auto fd = fds[id];
        // arbiter removes a tcp stream on error itself
        // if the client code also wants to withdraw
        // that's fine, it's a noop.
        if (fd != freed_id)
        {
            syscalls::epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
            next_timestamp[id] = time::max;
            --streams_under_arbitration;
            fds[id] = freed_id;
        }
    }
    void submission_complete() noexcept
    {
    }

    stream_id_t fd_to_id(int fd) noexcept
    {
        stream_id_t j = 0;
        for (; j != fds.size(); ++j)
        {
            if (fds[j] == fd)
            {
                // found the offset;
                return j;
            }
        }
        INVARIANT_MSG(j != fds.size(),
                      "invalid fd returned from epoll_wait "
                          << DUMP(fd) << DUMP(fds.size()));
        return arbiter_error::end;
    }

    void poll_all_descriptors() noexcept
    {
        if (num_ready)
        {
            return;
        }
        num_ready = epoll_wait(epfd, ev_list, max_events, 0);
        for (int i = 0; i != num_ready; ++i)
        {
            auto fd = ev_list[i].data.fd;
            auto id = fd_to_id(fd);
            next_timestamp[id] = clock.now();
            if (ev_list[i].events & (EPOLLHUP | EPOLLERR))
            {
                withdraw(id);
                ::close(fd);
                next_timestamp[id] = clock.now();
            }
        }
    }

    uint64_t winning_time(stream_id_t idx) noexcept
    {
        if (idx < next_timestamp.size())
        {
            return next_timestamp[idx];
        }
        return time::max;
    }
    stream_id_t ruling() noexcept
    {
        if (streams_under_arbitration == 0)
        {
            return arbiter_error::empty;
        }
        poll_all_descriptors();
        stream_id_t winner = arbiter_error::end;
        uint64_t winning_time = time::max;
        for (stream_id_t id = 0; id != next_timestamp.size(); ++id)
        {
            if (next_timestamp[id] < winning_time)
            {
                winning_time = next_timestamp[id];
                winner = id;
            }
        }
        // if something is ready we shouldn't be returning end
        ASSERT_MSG(
            !((num_ready != 0) && (winner == arbiter_error::end)),
            "reached end without calling poll() was a read_complete missed?");
        return winner;
    }
    void read_complete(stream_id_t idx)
    {
        next_timestamp[idx] = time::max;
        --num_ready;
        INVARIANT(num_ready >= 0);
    }

    void warm()
    {
    }

    fundamentals::vector<uint64_t> next_timestamp;
    fundamentals::vector<int> fds;
    static const int max_events = 8;
    struct epoll_event ev_list[max_events];
    Clock& clock;
    int num_ready;
    int epfd;
    int16_t streams_under_arbitration;
    static const int freed_id = -1;

  public:
    static const stream_id_t starting_offset;

} ALIGN(CACHE_LINE_SIZE);

template <typename Clock>
stream_id_t const kernel_arbiter<Clock>::starting_offset = 0;
} // namespace qstream
} // namespace miye
