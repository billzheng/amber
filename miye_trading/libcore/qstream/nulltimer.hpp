/*
 * nulltimer.hpp
 *
 * Purpose - timer that always returns time::max
 * placeholder in an arbiter if you don't want a timer at all.
 *
 *
 */
#pragma once
#include "libcore/time/time.hpp"
#include "qstream_reader_interface.hpp"

namespace miye
{
namespace qstream
{

template <typename Clock>
class nulltimer : public qstream_reader_interface<nulltimer<Clock>>
{
  public:
    nulltimer(Clock& clk_, const std::string description_ = "")
    {
        INVARIANT_MSG(!description_.size() || !description_.find("nulltimer"),
                      DUMP(description_));
    }
    ~nulltimer()
    {
    }
    void attest(uint64_t* next_timestamp)
    {
        *next_timestamp = time::max;
    }
    const place read()
    {
        return place::eof();
    }
    static const std::string description;
};

template <typename Clock>
const std::string nulltimer<Clock>::description = "nulltimer";

} // namespace qstream
} // namespace miye
