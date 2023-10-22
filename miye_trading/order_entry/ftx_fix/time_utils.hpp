#pragma once

#include <string>
#include <sys/time.h>
#include <ctime>
#include "libcore/time/gmtime.hpp"

namespace time_utils
{

inline std::string getTime()
{
    timeval t;
    gettimeofday(&t, NULL);
    auto milli = t.tv_usec / 1000;

    char buffer[0xFF] {};
    auto len = strftime(buffer, sizeof(buffer), "%Y%m%d-%H:%M:%S", qx::gmtime::gmtime(&t.tv_sec));
    sprintf(buffer + len, ".%03ld", milli);

    return std::string(buffer, len + 4);
}

inline void to_time_str(std::string& str, const std::chrono::high_resolution_clock::time_point& tp)
{
	using namespace std::chrono;

	auto const ms = duration_cast<milliseconds>(tp.time_since_epoch());
	auto const s = duration_cast<seconds>(ms);
	const std::time_t t = s.count();
	auto len = strftime((char*)str.data(), size_t(21), "%Y%m%d-%H:%M:%S", qx::gmtime::gmtime(&t));

	auto const milli = ms.count() % 1000;
	sprintf((char*)str.data() + len, ".%03ld", milli);
}

inline uint64_t now_ns()
{
    timespec ts{};
    if (::clock_gettime(CLOCK_REALTIME, &ts) == 0)
    {
        return ts.tv_sec * 1000'000'000 + ts.tv_nsec;
    }
    return 0;
}

} // namespace time_utils
