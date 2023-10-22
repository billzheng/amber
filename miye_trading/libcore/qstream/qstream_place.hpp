/*
 * qstream_place.hpp
 * definition of place return when you pledge to write to a stream
 * Author 
 */

#include "libcore/essential/platform_defs.hpp"
#include <stddef.h>
#pragma once

namespace miye
{
namespace qstream
{
struct place
{
    place() : start(0), size(0)
    {
    }
    place(char* start_, size_t size_) : start(start_), size(size_)
    {
    }
    place(void* start_, size_t size_) : start(start_), size(size_)
    {
    }

    static place eof()
    {
        return place((void*)nullptr, 0);
    }

    bool operator==(const place& rhs) const
    {
        return rhs.start == start && rhs.size == size;
    }
    bool operator!=(const place& rhs) const
    {
        return rhs.start != start || rhs.size != size;
    }
    bool is_eof() const
    {
        return *this == eof();
    }

    place pop_front(size_t to_remove) const
    {
        return (place((char*)start + to_remove, size - to_remove));
    }

    void* start;
    size_t size;
};

template <typename OS>
COLD OS& operator<<(OS& os, place const& p)
{
    os << "place(" << p.start << ", " << p.size << ")";
    return os;
}

} // namespace qstream
} // namespace miye
