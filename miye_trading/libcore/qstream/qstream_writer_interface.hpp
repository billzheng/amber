/*
 * qstream_interface.hpp
 * Purpose base object of CRTP defining the stream interface
 * Author 
 */

#include "qstream_place.hpp"
#include <string>
#pragma once

namespace miye
{
namespace qstream
{

#ifndef implementation
#define implementation static_cast<Derived*>(this)
#else
#error "implemenation is already defined"
#endif

#ifndef implementation_const
#define implementation_const static_cast<Derived const*>(this)
#else
#error "implemenation_const is already defined"
#endif

template <typename Derived>
class qstream_writer_interface
{
  public:
    const std::string& describe() const
    {
        return implementation_const->description;
    }

    place pledge(size_t n)
    {
        return implementation->pledge(n);
    }

    int announce(const place& p)
    {
        return implementation->announce(p);
    }

    int write(const void* p, size_t n)
    {
        auto plc = pledge(n);
        memcpy(plc.start, p, n);
        return announce(plc);
    }
};

#undef implementation
#undef implementation_const

} // namespace qstream
} // namespace miye
