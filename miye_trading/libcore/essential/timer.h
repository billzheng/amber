#pragma once

#include "date/date.h"
#include "libcore/utils/nano_time.h"

#include <chrono>
#include <iostream>

namespace miye
{

/*
 * use system clock instead of cpu cycles now
 */
struct TimeoutCB
{
    TimeoutCB() = default;

    virtual ~TimeoutCB() {}
    virtual int onTimeout() = 0;
    uint64_t nextWakeup() const { return nextTSC_; }

    bool timedOut() const
    {
        return time::NanoClock::now().time_since_epoch().count() > nextTSC_;
    }

  protected:
    uint64_t nextTSC_{static_cast<uint64_t>(-1)}; // next TSE to wake up
};

struct NanoTimerListener
{
    virtual ~NanoTimerListener() = default;
    virtual int onTimer(int32_t id, time::NanoTime scheduled) = 0;
};

struct Timer : public TimeoutCB
{
    static const constexpr std::chrono::microseconds PRECISION_DIFF{-20};

    explicit Timer(int32_t id) : id_(id) {}

    int onTimeout() override
    {
        auto const now = time::NanoClock::now();
        auto const diff = now - next_;
        if (diff > PRECISION_DIFF)
        {
            // we only update now if it has actually passed the deadline
            // allow one to be 20 usec early
            int ret = 0;
            //            std::cout
            //                << "timer expired"
            //                << date::format(
            //                       "%F %T",
            //                       std::chrono::time_point_cast<std::chrono::microseconds>(
            //                           now))
            //                << std::endl;

            if (nanoListener_)
            {
                ret = nanoListener_->onTimer(id_, next_);
            }
            next_ += delta_;
            if (next_ > end_)
            {
                std::cout << " next > end" << std::endl;
                return 0;
            }
        }
        else
        {
            updateTimeout(now);
            return 0;
        }
        return 0;
    }
    void setListener(NanoTimerListener* listener)
    {
        this->nanoListener_ = listener;
    }

    void updateTimeout(time::NanoTime now)
    {
        // set up for onTimeout, instead of simply incrementing nextTSC, we
        // recalibrate so that errors do not accumulate, not super fast but as
        // long as delta is greater than 10us this is negligible
        if (next_ < now)
        {
            nextTSC_ = 0;
        }
        else
        {
            auto diff = next_ - now;
            if (diff > std::chrono::seconds(30))
            {
                // do not wait for too long
                diff = std::chrono::seconds(30);
            }
            uint64_t now2 = time::NanoClock::now().time_since_epoch().count();
            uint64_t nanosecDiff = time::convertToNano(
                std::chrono::duration_cast<std::chrono::nanoseconds>(diff));
            nextTSC_ = now2 + nanosecDiff;
        }
    }

    int init(time::NanoTime start, time::NanoTime end, time::NanoDuration delta)
    {
        next_ = start + delta;
        end_ = end;
        delta_ = delta;
        return 0;
    }
    time::NanoTime nanoEnd() const { return end_; }
    time::NanoDuration nanoDelta() const { return delta_; }

  protected:
    NanoTimerListener* nanoListener_{nullptr};
    int32_t id_{};
    time::NanoTime next_{};
    time::NanoTime end_{};
    time::NanoDuration delta_{};
};

} // namespace miye
