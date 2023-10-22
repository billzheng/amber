/*
 * cycletimer.hpp
 *
 * Purpose - timer events based on rdtsc, can work with either real or discrete
 * clocks
 *
 *
 */

#pragma once
#include "libcore/essential/arch.hpp"
#include "libcore/essential/assert.hpp"
#include "libcore/time/rdtsc.hpp"
#include "libcore/time/time.hpp"
#include "qstream_common.hpp"
#include "qstream_reader_interface.hpp"
#include <limits.h>

namespace miye
{
namespace qstream
{

template <typename Clock>
class cycletimer : public qstream_reader_interface<cycletimer<Clock>>
{
  public:
    cycletimer(Clock& clk_, const std::string& description_)
        : clk(clk_), interval(0), timer_clock(0), description(description_)
    {
        last_read = essential::rdtscp();

        auto path = extract_path(description);
        interval = ::strtoull(path.c_str(), NULL, 10);
        if (!interval || (interval == ULLONG_MAX && errno == ERANGE))
        {
            INVARIANT_FAIL("no timer interval set");
        }
        cycle_interval = interval / time::calibrate_ticks();
    }

    ~cycletimer()
    {
    }

    void attest(uint64_t* next_timestamp)
    {
        // TODO make this branchless
        if (essential::rdtscp() - last_read > cycle_interval)
        {
            if (timer_clock <= this->clk.now() && this->clk.now() > 0)
            {
                timer_clock = this->clk.now() + interval;
                *next_timestamp = timer_clock;
            }
            else
            {
                *next_timestamp = time::max;
                timer_clock = this->clk.now();
            }
            last_read = essential::rdtscp();
        }
        else
        {
            *next_timestamp = time::max;
        }
    }
    const place read()
    {
        return place(&timer_clock, sizeof(timer_clock));
    }

    Clock& clk;

  private:
    uint64_t interval;
    uint64_t last_read;
    uint64_t timer_clock;
    uint64_t cycle_interval;

  public:
    const std::string description;
};

} // namespace qstream
} // namespace miye
