/*
 * qstream_reader_interface.hpp
 * Purpose base object of CRTP defining the stream reader interface
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
#error "implementation is already defined"
#endif

#ifndef implementation_const
#define implementation_const static_cast<Derived const*>(this)
#else
#error "implemenation_const is already defined"
#endif

template <typename Derived>
class qstream_reader_interface
{
  public:
    const std::string& describe() const
    {
        return implementation_const->description;
    }

    const place read()
    {
        return implementation->read();
    }

    // stream is called by arbiter to attest
    // returns timestamp of next read
    void attest(uint64_t* next_timestamp)
    {
        implementation->attest(next_timestamp);
    }
};
#undef implementation
#undef implementation_const
} // namespace qstream
} // namespace miye
