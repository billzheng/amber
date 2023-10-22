/*
 * arbiter.hpp
 * Purpose: arbitrate between mmaps in favour of lowest unread timestamp
 * Author: 
 */
#pragma once

#include "arbiter_common.hpp"
#include "libcore/essential/assert.hpp"
#include "libcore/essential/platform_defs.hpp"
#include "libcore/essential/utils.hpp"
#include "libcore/time/clock.hpp"
#include "libcore/utils/array.hpp"
#include "mmap_headers.hpp"
#include "nulltimer.hpp"

namespace miye
{
namespace qstream
{

template <typename Clock, typename Stream_t, int Capacity,
          typename Timer = nulltimer<Clock>>
class arbiter
{
    static_assert(Capacity < end, "Capacity is not less than end");

  public:
    explicit arbiter(Clock clock_)
        : clk(clock_), timer(clk),
          last_stream_index(1) // pos 0 reserved for timer
          ,
          submission_completed(false), streams_under_arbitration(0)
    {
        INVARIANT_ALIGNED(this, CACHE_LINE_SIZE);
        for (size_t i = 0; i != essential::static_max(Capacity + 1, 8); ++i)
        {
            next_timestamp[i] = nothing_to_read;
            free_ids[i] = arbiter_error::empty;
            streams[i] = nullptr;
        }
        INVARIANT_MSG(!timer.describe().compare("nulltimer"),
                      "must provide a timer description if you use one.");
    }
    explicit arbiter(Clock clock_, std::string timer_desc)
        : clk(clock_), timer(clk, timer_desc),
          last_stream_index(1) // pos 0 reserved for timer
          ,
          submission_completed(false), streams_under_arbitration(0)
    {
        INVARIANT_ALIGNED(this, CACHE_LINE_SIZE);
        for (size_t i = 0; i != essential::static_max(Capacity + 1, 8); ++i)
        {
            next_timestamp[i] = time::max;
            free_ids[i] = arbiter_error::empty;
            streams[i] = nullptr;
        }
    }

    Clock& clock()
    {
        return clk;
    }

    // checks the id reuturned is what is expected
    // matches the slow_arbiter api.
    stream_id_t submit(stream_id_t desired_id, Stream_t& stream) noexcept
    {
        auto returned_id = submit(&stream);
        INVARIANT_MSG(returned_id == desired_id,
                      "Lost count in arbiter submission "
                          << DUMP(+desired_id) << DUMP(+returned_id));
        return returned_id;
    }

    stream_id_t submit(Stream_t* stream_ptr) noexcept
    {
        // resubmission of withdrawn stream
        // give th same id back again
        for (stream_id_t id = 0; id != free_ids.size(); ++id)
        {
            if ((free_ids[id] != arbiter_error::empty) &&
                (streams[id] == stream_ptr))
            {
                INVARIANT_MSG(next_timestamp[id] == withdrawn_timestamp,
                              "resubmitting without withdraw"
                                  << DUMP(id) << stream_ptr->describe());
                free_ids[id] = arbiter_error::empty;
                next_timestamp[id] = recheck_timestamp;
                ++streams_under_arbitration;
                return id;
            }
        }
        // not a re-submission, find a new id for the stream
        // and set it up.

        // submit a stream for arbitration by the arbitrater
        // returns the index it will use to id
        // the stream when it reports arbitration
        stream_id_t this_stream_index = next_avail_stream_id();
        // pass a pointer to the memory we want the stream to
        // update with the timstamp of its next read
        streams[this_stream_index] = stream_ptr;

        ++streams_under_arbitration;
        return this_stream_index;
    }

    void withdraw(stream_id_t id) noexcept
    {
        // withdraws the stream with id from arbitration
        // eg because eof hit and not following
        --streams_under_arbitration;
        next_timestamp[id] = withdrawn_timestamp;
        return_freed_id(id);
        INVARIANT_MSG(streams_under_arbitration >= 0, "can't be negative");
    }
    void read_complete(stream_id_t idx)
    {
        next_timestamp[idx] = recheck_timestamp;
    }

    stream_id_t min_of_2(stream_id_t i, stream_id_t j) noexcept
    {
        // branchless and takes advantage of ILP. Might be fast - need to
        // benchmark
        // !! below ensures logical test returns either 0 or 1 (rather than 0 or
        // not zero) min_i_j returns 1 if timestamp at i is higher than at j
        // -1ns below is so we don't claim that a withdrawn_timestamp
        // beats nothing_to_read, if 2 streams are 1 ns apart we may
        // treat them as equal and service in arbitrary order.
        ASSERT(next_timestamp[i] != 0);

        uint8_t min_i_j = (!((next_timestamp[i] - 1) < next_timestamp[j]));
        // mask is either 0 (when i first) or 0xff (when j first),
        uint8_t mask = -min_i_j;
        return i ^ ((i ^ j) &
                    mask); // mask is 0=> i ^ 0 == i, mask is 0xff i^(i^j) == j
    }
    void submission_complete()
    {
        INVARIANT_MSG(
            Capacity >= 8 || last_stream_index == Capacity + 1,
            "arbiter Capacity must be tne number of streams actually added. "
                << DUMP(Capacity) << " + 1 "
                << "!=" << DUMP(last_stream_index));
        submission_completed = true;
    }

    uint64_t winning_time(stream_id_t idx) noexcept
    {
        if (idx < next_timestamp.size())
        {
            return next_timestamp[idx];
        }
        return time::max;
    }

    stream_id_t slow_ruling() noexcept
    {
        if (streams_under_arbitration == 0)
        {
            return arbiter_error::empty;
        }
        timer.attest(&next_timestamp[0]);
        stream_id_t winner = 0;
        for (int i = 1; i != last_stream_index; ++i)
        {
            streams[i]->slow_attest(&next_timestamp[i]);
        }
        for (size_t i = 0; i < last_stream_index; ++i)
        {
            if (next_timestamp[i] < withdrawn_timestamp &&
                next_timestamp[i] < next_timestamp[winner])
            {
                winner = i;
            }
        }
        if (next_timestamp[winner] == time::max)
        {
            return arbiter_error::end;
        }
        return winner;
    }

    stream_id_t ruling() noexcept
    {
        timer.attest(&next_timestamp[0]);
        stream_id_t winner = 0;

        if (Capacity < 8)
        {
            ASSERT(last_stream_index == Capacity + 1);
            ASSERT_MSG(submission_completed,
                       "must call submission_complete() on the arbiter after "
                       "loading all the streams");
            // this should be branchless with the if statements collapsing
            // when Capacity is known at compile time
            // all code to handle a number of streams greater than Capacity
            // should be removed by the compiler as it is known to be dead code
            // at compile time

            // index 0 is reserved for the timer
#define CHECK_AT_IDX(idx)                                                      \
    if (Capacity >= idx)                                                       \
        do                                                                     \
        {                                                                      \
            streams[idx]->attest(&next_timestamp[idx]);                        \
    } while (0)
            CHECK_AT_IDX(1);
            CHECK_AT_IDX(2);
            CHECK_AT_IDX(3);
            CHECK_AT_IDX(4);
            CHECK_AT_IDX(5);
            CHECK_AT_IDX(6);
            CHECK_AT_IDX(7);
#undef CHECK_AT_IDX

            // ILP version, should be better than avx2 for 8 * 64bit timestamp
            // compare possibly revisit for more than 8 streams only 1 stream,
            // first is always the winner

            if (Capacity == 1)
            {
                winner = min_of_2(0, 1);
            }
            else if (Capacity <= 3)
            {
                // 2 or 3 streams
                stream_id_t a = min_of_2(0, 1);
                stream_id_t b = min_of_2(2, 3);
                winner = min_of_2(a, b);
            }
            else if (Capacity >= 4)
            {
                // 4 to 7 streams
                stream_id_t a = min_of_2(0, 1);
                stream_id_t b = min_of_2(2, 3);
                stream_id_t c = min_of_2(4, 5);
                stream_id_t d = min_of_2(6, 7);

                stream_id_t e = min_of_2(a, b);
                stream_id_t f = min_of_2(c, d);
                winner = min_of_2(e, f);
            }
        }

        // slow version if we have more streams
        if (Capacity >= 8)
        {
            // probably no need to optimise further with >8 mmap streams in an
            // arbiter this block should also be removed as dead at compile time
            // if not needed
            for (size_t i = 1; i < last_stream_index; ++i)
            {
                streams[i]->attest(&next_timestamp[i]);
            }
            // include the timer stream to see who wins
            for (size_t i = 0; i < last_stream_index; ++i)
            {
                if ((next_timestamp[i] < withdrawn_timestamp) &&
                    (next_timestamp[i] < next_timestamp[winner]))
                {
                    winner = i;
                }
            }
        }

        // if the winnner is time::max, return end
        // 1 or 0
        uint64_t winner_is_time_max = !(next_timestamp[winner] != time::max);
        // mask all 1s when winner is time max;
        uint64_t mask = -(winner_is_time_max);
        // returns arbiter_error::end with mask all 1s ortherwise winner
        auto retval = winner ^ ((winner ^ arbiter_error::end) & mask);

        // if all streams have been withdrawn return empty
        return retval ^ ((retval ^ arbiter_error::empty) &
                         -(!streams_under_arbitration));
    }

    stream_id_t next_avail_stream_id() noexcept
    {

        stream_id_t this_stream_index = 1;
        size_t i = 0;
        for (; i != Capacity; ++i)
        {
            if (free_ids[i] != arbiter_error::empty)
            {
                this_stream_index = free_ids[i];
                free_ids[i] = arbiter_error::empty;
                break;
            }
        }
        if (i == Capacity)
        {
            this_stream_index = last_stream_index;
            ++last_stream_index;
        }
        // Capacity 2, has 3 slots (timer is slot 0) last_stream_index ends up
        // being the index just past the end
        INVARIANT_MSG(last_stream_index <= Capacity + 1,
                      "overfilled arbiter, Capacity + 1 ="
                          << Capacity + 1 << " " << DUMP(+last_stream_index));
        return this_stream_index;
    }

    void return_freed_id(stream_id_t id)
    {
        // id 0 is never freed - always for the timer
        if (id == 0)
        {
            return;
        }
        for (size_t i = 0; i != Capacity; ++i)
        {
            if (free_ids[i] == arbiter_error::empty)
            {
                free_ids[i] = id;
                break;
            }
        }
    }

    // frequently used, latency critical access
    // stores next read timestamp for each stream
    fundamentals::array<uint64_t, essential::static_max(Capacity + 1, 8)>
        next_timestamp;
    fundamentals::array<Stream_t*, essential::static_max(Capacity + 1, 8)>
        streams;
    // is clock latency critical? probably
    Clock clk;
    Timer timer;

    // infrequently used, non latency critical access
    stream_id_t last_stream_index;
    bool submission_completed;
    int16_t streams_under_arbitration;
    fundamentals::array<stream_id_t, essential::static_max(Capacity + 1, 8)>
        free_ids;
    static const stream_id_t freed_id = arbiter_error::empty;

  public:
    static const stream_id_t starting_offset;
} ALIGN(CACHE_LINE_SIZE);

template <typename Clock, typename Stream_t, int Capacity, typename Timer>
stream_id_t const arbiter<Clock, Stream_t, Capacity, Timer>::starting_offset =
    1;

} // namespace qstream
} // namespace miye
