/*
 * rdtsc.hpp
 *
 * Purpose: read the time stamp counter
 *
 * Author: 
 */


#pragma once

#include <stdint.h>
#include <time.h>
#include "libcore/essential/arch.hpp"
#include "libcore/time/timeutils.hpp"

namespace miye { namespace time {

using essential::rdtscp;

inline double calibrate_ticks()
{
    struct timespec ts1, ts2;
    uint64_t        nanos;
    uint64_t        s, e;

    ::clock_gettime(CLOCK_REALTIME, &ts1);
    s = rdtscp();
    for (volatile uint64_t i = 0, j = 0; i < 10000000; i++) {
        j += i;
    }
    e = rdtscp();
    ::clock_gettime(CLOCK_REALTIME, &ts2);
    nanos = (ts2.tv_sec * time::seconds(1) + ts2.tv_nsec) - (ts1.tv_sec * time::seconds(1) + ts1.tv_nsec);
    return (double)nanos / (e - s);
}

struct cycle_count
{
    cycle_count(uint64_t& c)
    : start(rdtscp())
    , count(c)
    {}

    ~cycle_count()
    {
        count = rdtscp() - start;
    }
private:
    uint64_t start;
    uint64_t& count;
};

class rdtsc_stopwatch
{
public:
    rdtsc_stopwatch()
    : ticks2ns_(time::calibrate_ticks())
    , last_(rdtscp())
    {}

    void reset()
    {
        last_ = rdtscp();
    }

    uint64_t elapsed_ns() const
    {
        return (rdtscp() - last_) * ticks2ns_;
    }

    uint64_t elapsed_cycles() const
    {
        return rdtscp() - last_;
    }

    double ticks2ns() const
    {
        return ticks2ns_;
    }

    uint64_t reset_and_elapsed()
    {
        uint64_t t((rdtscp() - last_) * ticks2ns_);
        last_ = rdtscp();
        return t;
    }

    uint64_t cycles() const
    {
        return rdtscp();
    }

    uint64_t wait(uint64_t period)
    {
        uint64_t elapsed = 0;
        reset();
        while(elapsed < period){ elapsed = elapsed_ns();}
        return elapsed;
    }

private:
    double ticks2ns_;
    uint64_t last_;
};


}}
