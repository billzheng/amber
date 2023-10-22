/*
 * timeutils.hpp
 *
 * Purpose: time utilities
 *
 * Author: 
 */

#pragma once

#include <iomanip>
#include <ctime>

#include "libcore/types/types.hpp"
#include "libcore/essential/platform_defs.hpp"
#include "gmtime.hpp"

namespace miye { namespace time {

inline constexpr uint64_t nanos(int amount) { return amount; }
inline constexpr uint64_t micros(int amount) { return 1000*nanos(amount); }
inline constexpr uint64_t millis(int amount) { return 1000*micros(amount); }
inline constexpr uint64_t seconds(int amount) { return 1000*millis(amount); }
inline constexpr uint64_t minutes(int amount) { return 60*seconds(amount); }
inline constexpr uint64_t hours(int amount) { return 60*minutes(amount); }
inline constexpr uint64_t days(int amount) { return 24*hours(amount); }
inline constexpr uint64_t weeks(int amount) { return 7*days(amount); }

inline constexpr uint64_t to_nanos(uint64_t nanos) { return nanos; }
inline constexpr uint64_t to_micros(uint64_t nanos) { return to_nanos(nanos) / 1000; }
inline constexpr uint64_t to_millis(uint64_t nanos) { return to_micros(nanos) / 1000; }
inline constexpr uint64_t to_seconds(uint64_t nanos) { return to_millis(nanos) / 1000; }
inline constexpr uint64_t to_minutes(uint64_t nanos) { return to_seconds(nanos) / 60; }
inline constexpr uint64_t to_hours(uint64_t nanos) { return to_minutes(nanos) / 60; }
inline constexpr uint64_t to_days(uint64_t nanos) { return to_hours(nanos) / 24; }
inline constexpr uint64_t to_weeks(uint64_t nanos) { return to_days(nanos) / 7; }


enum weekday_t
{
    sunday = 0,//note not ISO standard, which is 1
    monday,
    tuesday,
    wednesday,
    thursday,
    friday,
    saturday
};

enum month_t
{
    january = 0,//note not ISO standard, which is 1
    februrary,
    march,
    april,
    may,
    june,
    july,
    august,
    september,
    october,
    november,
    december
};
inline uint64_t to_nanos(std::string const& str)
{
    tm tm_;
    memset(&tm_, 0, sizeof(tm));
    tm_.tm_isdst = -1;

    const char* s = nullptr;
    if (str.size() == 8)
        s = strptime(str.c_str(), "%Y%m%d", &tm_);
    else if (str.size() >= 17)
        s = strptime(str.c_str(), "%Y%m%dT%H:%M:%S", &tm_);
    else
        INVARIANT_MSG(false, "Invalid time/date string: " << str);

    if (!s)
        INVARIANT_MSG(false, "Invalid time/date string: " << str);

    uint64_t nanos = 0;
    if (size_t(s - str.c_str()) < str.size() && *s == '.')
    {
        ++s;
        char* s1;
        nanos = strtol(s, &s1, 10);
        if (s1 != s + 9)
            INVARIANT_MSG(false, "Invalid time/date input. Must have 9 digits of fractional seconds: " << str);
        s = s1;
    }

    bool include_utc_offset = true;
    if (size_t(s - str.c_str()) != str.size())
    {
        if (*s != 'Z')
            INVARIANT_MSG(false, "Invalid time/date input. Unexpected trailing characters: " << str);
    }
    time_t tm = mktime(&tm_);
    return seconds(tm) + nanos + (include_utc_offset ? seconds(tm_.tm_gmtoff) : 0);
}

inline uint64_t system_nanos()
{
    struct timespec ts;
    ::clock_gettime(CLOCK_REALTIME, &ts);
    return (ts.tv_sec * time::seconds(1) + ts.tv_nsec);
}

struct as_utc
{
    explicit as_utc(uint64_t t)
    : time(t)
    {}
    uint64_t time;
};

template<typename OStream>
inline OStream& write_time(OStream& os, struct tm &tm_, long nanos)
{
    char                buffer[64];
    size_t              n;

    n = ::strftime(buffer, sizeof(buffer), "%Y%m%dT%H:%M:%S", &tm_);
    ASSERT(n > 0);
    ASSERT(n < 64);
    buffer[n] = '\0';

    os << buffer << "."
       << std::setfill('0')
       << std::setw(9) << nanos;
    if (tm_.tm_gmtoff == 0)
    {
        os << "Z";
    }
    else
    {
        if (tm_.tm_gmtoff > 0)
            os << "+";
        else
            os << "-";
        os << std::setfill('0')
           << std::setw(2)
           << tm_.tm_gmtoff / 60
           << ":"
           << std::setfill('0')
           << std::setw(2)
           << tm_.tm_gmtoff % 60;
    }

    return os;
}


template<typename OStream>
OStream& operator<<(OStream& os, as_utc const& t)
{
    struct tm           tm_;
    time_t              secs;
    long                nanos;

    secs = t.time / time::seconds(1);
    nanos = t.time % time::seconds(1);

    gmtime::gmtime_r(&secs, &tm_);

    return write_time(os, tm_, nanos);
}


struct as_local
{
    explicit as_local(uint64_t t)
    : time(t)
    {}
    uint64_t time;
};


template<typename OStream>
OStream& operator<<(OStream& os, as_local const& t)
{
    struct tm           tm_;
    time_t              secs;
    long                nanos;

    secs = t.time / time::seconds(1);
    nanos = t.time % time::seconds(1);

    ::localtime_r(&secs, &tm_);

    return write_time(os, tm_, nanos);
}


struct as_hmsm
{
    explicit as_hmsm(uint64_t t)
    : time(t)
    {}
    uint64_t time;
};


template<typename OStream>
OStream& operator<<(OStream& os, as_hmsm const& t)
{
    struct tm           tm_;
    time_t              secs;
    long                milli_secs;
    char                buffer[64];
    size_t              n;
    std::ostringstream  s;

    secs = t.time / time::seconds(1);
    milli_secs = t.time / time::millis(1);
    milli_secs = milli_secs - secs * 1000;

    gmtime::gmtime_r(&secs, &tm_);

    UNUSED(n);
    n = ::strftime(buffer, sizeof(buffer), "%H:%M:%S", &tm_);
    ASSERT(n > 0);

    s << buffer << "." << std::setfill('0') << std::setw(3) << milli_secs;

    os << s.str();

    return os;
}

inline std::string format_nano_interval(uint64_t nanos)
{
    std::ostringstream oss;
    if (nanos >= time::hours(1))
    {
        oss << double(nanos) / time::hours(1) << "h";
    }
    else if (nanos >= time::minutes(1))
    {
        oss << double(nanos) / time::minutes(1) << "m";
    }
    else if (nanos >= time::seconds(1))
    {
        oss << double(nanos) / time::seconds(1) << "s";
    }
    else if (nanos >= time::millis(1))
    {
        oss << double(nanos) / time::millis(1) << "ms";
    }
    else if (nanos >= time::micros(1))
    {
        oss << double(nanos) / time::micros(1) << "us";
    }
    else if (nanos >= time::nanos(1))
    {
        oss << nanos << "ns";
    }
    else
        oss << "0 ns";
    return oss.str();
}

inline uint64_t parse_nano_interval(std::istringstream & iss)
{
    double interval;
    std::string units;
    iss >> interval >> units;
    if (units == "h")
    {
        return uint64_t(interval*hours(1));
    }
    else if (units == "m")
    {
        return uint64_t(interval*minutes(1));
    }
    else if (units == "s")
    {
        return uint64_t(interval*seconds(1));
    }
    else if (units == "ms")
    {
        return uint64_t(interval*millis(1));
    }
    else if (units == "us")
    {
        return uint64_t(interval*micros(1));
    }
    else if (units == "ns")
    {
        return uint64_t(interval*nanos(1));
    }
    return uint64_t(interval*nanos(1));
}



inline uint64_t parse_nano_interval(std::string const& str)
{
    std::istringstream iss(str);
    return parse_nano_interval(iss);
}

inline uint64_t convert_to_utc(std::string timestr, std::string from_tz)
{
    // timestr = YYYYMMDDTHH:MM:SS

    tm tm_;
    memset(&tm_, 0, sizeof(tm));
    tm_.tm_isdst = -1;

    strptime(timestr.c_str(), "%Y%m%dT%H:%M:%S", &tm_);

    setenv("TZ", from_tz.c_str(), 1);
    tzset();
    time_t t1 = mktime(&tm_);
    uint64_t ts = time::seconds(t1);
    return ts;
}

inline time_t convert_from_utc(time_t time_utc, std::string to_tz)
{
    std::ostringstream oss;
    oss << as_utc(time_utc);
    tm time_in = {stoi(oss.str().substr(15,2)), stoi(oss.str().substr(12,2)), stoi(oss.str().substr(9,2)), stoi(oss.str().substr(6,2)), stoi(oss.str().substr(4,2)) -1, stoi(oss.str().substr(0,4)) - 1900};
    time_in.tm_isdst = -1;
    setenv("TZ", to_tz.c_str(), 1);
    tzset();
    return time_utc - (mktime(&time_in) * time::seconds(1) - time_utc);
}

inline bool operator==(struct tm const& l, struct tm const& r)
{
    return l.tm_gmtoff == r.tm_gmtoff
        && l.tm_hour == r.tm_hour
        && l.tm_isdst == r.tm_isdst
        && l.tm_mday == r.tm_mday
        && l.tm_min == r.tm_min
        && l.tm_mon == r.tm_mon
        && l.tm_sec == r.tm_sec
        && l.tm_wday == r.tm_wday
        && l.tm_yday == r.tm_yday
        && l.tm_year == r.tm_year;
}

template<typename OStream>
OStream& operator<<(OStream& os, tm const& t)
{
    os << DUMP(t.tm_gmtoff) << ", " << DUMP(t.tm_hour) << ", " << DUMP(t.tm_isdst) << ", " << DUMP(t.tm_mday) << ", " << DUMP(t.tm_min) << ", " << DUMP(t.tm_mon) << ", " << DUMP(t.tm_sec) << ", " << DUMP(t.tm_wday) << ", " << DUMP(t.tm_yday) << ", " << DUMP(t.tm_year);
    return os;
}




}}
