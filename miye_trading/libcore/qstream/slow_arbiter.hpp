/*
 * arbiter.hpp
 * Purpose: arbitrate between streams in favour of lowest unread timestamp
 * Author: 
 */
#pragma once

#include "arbiter.hpp"
#include "arbiter_common.hpp"
#include "kernel_arbiter.hpp"
#include "libcore/essential/assert.hpp"
#include "libcore/essential/platform_defs.hpp"
#include "libcore/essential/utils.hpp"
#include "libcore/time/clock.hpp"
#include "libcore/utils/array.hpp"
#include "mmap_reader.hpp"
#include "variantqstream_reader.hpp"

namespace miye
{
namespace qstream
{

template <typename Clock, uint8_t Mmap_Capacity = 16,
          typename Timer = nulltimer<Clock>>
class slow_arbiter
{
    typedef mmap_reader<Clock> mmap_reader_t;
    typedef variantqstream_reader<Clock> variantq_reader_t;

  public:
    explicit slow_arbiter(Clock& clock_) noexcept
        : marb(clock_), karb(clock_), clk(clock_)
    {
        INVARIANT_ALIGNED(this, CACHE_LINE_SIZE);
        static_assert(Mmap_Capacity > 8,
                      "Mmap_Capacity must be at least 8 for a slow arbiter");
    }
    // if you have a timer, the stream id that is returned when it fires is
    // arbiter_error::timeout
    slow_arbiter(Clock& clock_, std::string timer_desc) noexcept
        : marb(clock_, timer_desc), karb(clock_), clk(clock_)
    {
        INVARIANT_ALIGNED(this, CACHE_LINE_SIZE);
        static_assert(Mmap_Capacity > 8,
                      "Mmap_Capacity must be at least 8 for a slow arbiter");
    }

    Clock& clock() noexcept
    {
        return clk;
    }

    stream_id_t submit(stream_id_t desired_id,
                       variantq_reader_t& stream) noexcept
    {
        // only the last place a variantqstream is copied is valid
        INVARIANT_MSG(stream.is_valid(),
                      "You're submitting a stream that has been copied to a "
                      "different location.");
        submit(stream, desired_id);
        return desired_id;
    }

    void withdraw(stream_id_t id) noexcept
    {
        // INVARIANT_MSG(public_to_unified.find(id) != public_to_unified.end(),
        // DUMP(+id) << " not a slow arbiter public stream id");
        if (public_to_unified.find(id) == public_to_unified.end())
        {
            return;
        }
        auto unified_id = public_to_unified[id];
        if (is_karb_unified_id(unified_id))
        {
            auto karb_id = unified_id_to_karb_id(unified_id);
            karb.withdraw(karb_id);
        }
        else
        {
            INVARIANT_MSG(is_marb_unified_id(unified_id), DUMP(unified_id));
            auto marb_id = unified_id_to_marb_id(unified_id);
            marb.withdraw(marb_id);
        }
        remove_public_mappings(id);
    }

    void read_complete(stream_id_t idx) noexcept
    {
        if (public_to_unified.find(idx) == public_to_unified.end())
        {
            std::cerr << DUMP(+idx) << " not a slow arbiter public stream id"
                      << std::endl;
            return;
        }
        auto unified_id = public_to_unified[idx];
        if (is_karb_unified_id(unified_id))
        {
            auto karb_id = unified_id_to_karb_id(unified_id);
            karb.read_complete(karb_id);
        }
        else
        {
            marb.read_complete(unified_id);
        }
    }

    stream_id_t ruling() noexcept
    {
        stream_id_t best_mmap = marb.slow_ruling();
        stream_id_t best_polled = karb.ruling();

        if (best_mmap == 0)
        {
            marb.read_complete(0);
            return arbiter_error::timeout;
        }
        // return empty if both return empty
        // (empty,end),(end,empty),(end,end)
        // all return end
        if (best_mmap == arbiter_error::empty)
        {
            if (best_polled == arbiter_error::empty)
            {
                return arbiter_error::empty;
            }
            else if (best_polled == arbiter_error::end)
            {
                return arbiter_error::end;
            }
        }
        else if (best_mmap == arbiter_error::end)
        {
            if (best_polled == arbiter_error::end ||
                best_polled == arbiter_error::empty)
            {
                return arbiter_error::end;
            }
        }

        auto mm_winner = marb.winning_time(best_mmap);
        auto k_winner = karb.winning_time(best_polled);
        auto winner = std::min(mm_winner, k_winner);
        if (winner == mm_winner)
        {
            auto unified_id = marb_id_to_unified(best_mmap);
            auto public_id = unified_to_public[unified_id];
            return public_id;
        }
        else
        {
            INVARIANT(winner == k_winner);
            auto unified_id = karb_id_to_unified(best_polled);
            auto public_id = unified_to_public[unified_id];
            return public_id;
        }
    }

    void submission_complete() noexcept
    {
        // unnecessary for a slow arbiter
    }

  private:
    void create_public_mappings(stream_id_t public_id,
                                stream_id_t unified_id) noexcept
    {

        INVARIANT_MSG(
            public_to_unified.find(public_id) == public_to_unified.end(),
            DUMP((int)public_id)
                << " already in use as a slow arbiter public stream id");
        INVARIANT_MSG(
            unified_to_public.find(unified_id) == unified_to_public.end(),
            DUMP((int)unified_id)
                << " already in use as a slow arbiter public unified id");
        public_to_unified[public_id] = unified_id;
        unified_to_public[unified_id] = public_id;
    }
    void remove_public_mappings(stream_id_t public_id) noexcept
    {
        auto unified_id = public_to_unified[public_id];
        public_to_unified.erase(public_id);
        unified_to_public.erase(unified_id);
    }
    stream_id_t submit(variantq_reader_t& stream,
                       stream_id_t desired_id) noexcept
    {
        stream_id_t unified_id;
        if (stream.is_kernel())
        {
            auto fd = stream.get_fd();
            unified_id = submit(fd);
        }
        else
        {
            unified_id = marb.submit(&stream);
        }
        if (desired_id == static_cast<stream_id_t>(-1))
        {
            desired_id = unified_id;
        }

        create_public_mappings(desired_id, unified_id);
        return desired_id;
    }
    // syscall free streams
    stream_id_t submit(mmap_reader_t& stream) noexcept
    {
        auto id = marb.submit(&stream);
        auto unified_id = marb_id_to_unified(id);
        return unified_id;
    }
    // streams requiring the kernel
    stream_id_t submit(int fd) noexcept
    {
        auto id = karb.submit(fd);
        auto unified_id = karb_id_to_unified(id);
        return unified_id;
    }

    stream_id_t marb_id_to_unified(stream_id_t marb_id)
    {
        return marb_id;
    }
    stream_id_t karb_id_to_unified(stream_id_t karb_id)
    {
        return karb_id + Mmap_Capacity;
    }

    stream_id_t unified_id_to_marb_id(stream_id_t unified_id)
    {
        return unified_id;
    }
    stream_id_t unified_id_to_karb_id(stream_id_t unified_id)
    {
        INVARIANT_MSG(is_karb_unified_id(unified_id),
                      DUMP(unified_id)
                          << " submitted as a kernel arbiter id - it isn't");
        return unified_id - Mmap_Capacity;
    }
    bool is_marb_unified_id(stream_id_t unified_id)
    {
        return unified_id < Mmap_Capacity;
    }
    bool is_karb_unified_id(stream_id_t unified_id)
    {
        return unified_id >= Mmap_Capacity;
    }

    arbiter<Clock, variantqstream_reader<Clock>, Mmap_Capacity, Timer> marb;
    kernel_arbiter<Clock> karb;
    Clock& clk;
    std::unordered_map<stream_id_t, stream_id_t> public_to_unified;
    std::unordered_map<stream_id_t, stream_id_t> unified_to_public;

  public:
    static const stream_id_t starting_offset;

} ALIGN(CACHE_LINE_SIZE);

template <typename Clock, uint8_t Mmap_Capacity, typename Timer>
stream_id_t const slow_arbiter<Clock, Mmap_Capacity, Timer>::starting_offset =
    0;

} // namespace qstream
} // namespace miye
