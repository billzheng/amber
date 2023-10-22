#pragma once
#include <chrono>
#include <cstdint>
#include <sys/time.h> // timeval
#include <time.h>     // timespec

namespace miye
{
namespace time
{

struct NanoClock
{
    using duration = std::chrono::nanoseconds;
    using period = duration::period;
    using rep = duration::rep;
    using time_point =
        std::chrono::time_point<std::chrono::system_clock, duration>;
    static time_point now();
};
using NanoTime = NanoClock::time_point;
using NanoDuration = NanoClock::duration;

struct NanoTimeUtil
{
    static NanoTime fromTimeSpec(const timespec tv);
};

inline NanoTime NanoClock::now()
{
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return NanoTimeUtil::fromTimeSpec(ts);
}

inline NanoTime NanoTimeUtil::fromTimeSpec(const timespec ts)
{
    return NanoTime(std::chrono::seconds(ts.tv_sec) +
                    std::chrono::nanoseconds(ts.tv_nsec));
}

template <typename Duration>
inline constexpr uint64_t convertToNano(Duration duration)
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(duration)
        .count();
}

} // namespace time
} // namespace miye
