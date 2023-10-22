/*
 * timer.hpp
 *
 * Purpose: timer i/o resource
 *
 */

#pragma once
#include "libcore/essential/assert.hpp"
#include "libcore/time/time.hpp"
#include "qstream_common.hpp"
#include "qstream_reader_interface.hpp"
#include <limits.h>

namespace miye
{
namespace qstream
{

template <typename Clock>
class timer : public qstream_reader_interface<timer<Clock>>
{
  public:
    timer(Clock& clk_, const std::string& description_)
        : clk(clk_), description(description_), interval(0), next(0), last(0)
    {
        auto path = extract_path(description);

        interval = ::strtoull(path.c_str(), NULL, 10);
        if (!interval || (interval == ULLONG_MAX && errno == ERANGE))
        {
            INVARIANT_MSG(false, "no timeout set");
        }
        last = this->clk.now();
        next = last + interval;
    }

    ~timer()
    {
    }

    void attest(uint64_t* next_timestamp)
    {
        uint64_t now = this->clk.now();

        if (now >= next)
        {
            *next_timestamp = now;
        }
    }

    const place read()
    {
        last = this->clk.now();
        next = last + interval;
        return place(&last, sizeof(last));
    }

    Clock& clk;
    const std::string description;

  private:
    uint64_t interval;
    uint64_t next;
    uint64_t last;
};

} // namespace qstream
} // namespace miye
