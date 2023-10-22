/*
 * arbiter_common.hpp
 * Purpose: common declarations between arbiters
 * Author: 
 */
#pragma once
#include "libcore/time/clock.hpp"
#include "libcore/time/time.hpp"

namespace miye
{
namespace qstream
{

typedef uint8_t stream_id_t;

static const uint64_t recheck_timestamp = time::max;
static const uint64_t nothing_to_read = time::max;
static const uint64_t withdrawn_timestamp = (time::max - 1);

enum arbiter_error
{
    timeout = static_cast<stream_id_t>(-1),
    interrupt = static_cast<stream_id_t>(-2),
    end = static_cast<stream_id_t>(-3),
    empty = static_cast<stream_id_t>(-4)
};

template <typename OS>
OS& operator<<(OS& os, arbiter_error e)
{
    switch (e)
    {
    case arbiter_error::timeout:
        os << "timeout";
        break;
    case arbiter_error::interrupt:
        os << "interrupt";
        break;
    case arbiter_error::end:
        os << "end";
        break;
    case arbiter_error::empty:
        os << "empty";
        break;
    }
    return os;
}

} // namespace qstream
} // namespace miye
