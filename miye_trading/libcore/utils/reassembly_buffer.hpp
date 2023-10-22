/*
 * reassembly_buffer.hpp
 * Purpose: convenience, array wrapper that implements append to back
 * and remove from front, shuffling data down.
 * Useful for partial tcp receives
 * Author: 
 */

#include "libcore/essential/assert.hpp"
#include "libcore/essential/platform_defs.hpp"
#include "libcore/utils/array.hpp"

#pragma once

namespace miye
{
namespace fundamentals
{

template <int Capacity>
struct reassembly_buffer
{

    fundamentals::array<char, Capacity> rec_buf;
    char* rec_buf_free_start;

    reassembly_buffer()
    {
        rec_buf_free_start = &rec_buf[0];
    }

    reassembly_buffer(const reassembly_buffer& rhs)
    {
        for (size_t i = 0; i != Capacity; ++i)
        {
            rec_buf[i] = rhs.rec_buf[i];
        }
        auto used = rhs.buffer_used();
        rec_buf_free_start = &rec_buf[used];
        ASSERT(buffer_used() == used);
    }

    reassembly_buffer& operator=(const reassembly_buffer& rhs)
    {
        for (size_t i = 0; i != Capacity; ++i)
        {
            rec_buf[i] = rhs.rec_buf[i];
        }
        auto used = rhs.buffer_used();
        rec_buf_free_start = &rec_buf[used];
        ASSERT(buffer_used() == used);
        return *this;
    }

    void copy_to_back(char* to_copy, int requested_size)
    {
        size_t already_used = this->buffer_used();
        INVARIANT_MSG((requested_size + already_used) <= Capacity,
                      DUMP(already_used)
                          << DUMP(requested_size) << DUMP(Capacity));
        memcpy(rec_buf_free_start, to_copy, requested_size);
        rec_buf_free_start += requested_size;
    }

    size_t buffer_used() const
    {
        INVARIANT_MSG(rec_buf_free_start >= &rec_buf[0],
                      DUMP((void*)rec_buf_free_start)
                          << DUMP((void*)&rec_buf[0]));
        size_t used = rec_buf_free_start - &rec_buf[0];
        return used;
    }

    void remove_from_front(int num_to_remove)
    {
        int total_used = buffer_used();
        ASSERT_MSG(total_used >= num_to_remove,
                   DUMP(total_used) << DUMP(num_to_remove));
        uint64_t remaining = total_used - num_to_remove;
        memmove(&rec_buf[0], &rec_buf[num_to_remove], remaining);
        rec_buf_free_start = &rec_buf[remaining];
        ASSERT_MSG(buffer_used() == remaining,
                   DUMP(buffer_used()) << DUMP(remaining));
    }

    void clear()
    {
        remove_from_front(buffer_used());
        ASSERT(rec_buf_free_start == &rec_buf[0]);
        ASSERT_MSG(buffer_used() == 0, DUMP(buffer_used()));
    }

    char const* data() const
    {
        return &rec_buf[0];
    }
    char* data()
    {
        return &rec_buf[0];
    }
};

} // namespace fundamentals
} // namespace miye
