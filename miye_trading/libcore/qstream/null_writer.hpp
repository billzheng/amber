/*
 * null_writer.hpp
 * purpose: implement the qstream_writer interface
 * but throw all the data away
 * Author: 
 */

#pragma once
#include "qstream_common.hpp"
#include "qstream_writer_interface.hpp"

namespace miye
{
namespace qstream
{

static const std::string null_description("null:");

template <typename Clock>
class null_writer : public qstream_writer_interface<null_writer<Clock>>
{
  public:
    null_writer(Clock& clock_, std::string descrip) : clock(clock_)
    {
        UNUSED(descrip);
    }
    const std::string& describe() const
    {
        return null_description;
    }
    place pledge(size_t n)
    {
        return place(writebuffer, n);
    }
    int announce(const place& p)
    {
        UNUSED(p);
        return 0;
    }
    Clock& clock;
    char writebuffer[SUPERPAGE_SIZE];
};

} // namespace qstream
} // namespace miye
