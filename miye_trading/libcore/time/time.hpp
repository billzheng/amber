/*
 * time.hpp
 *
 * Purpose: time of day functions
 *
 * Author: 
 */

#pragma once

#include "libcore/parsing/json.hpp"
#include "libcore/time/timeutils.hpp"
#include "libcore/time/gmtime.hpp"
#include <sstream>
#include "libcore/essential/assert.hpp"

namespace miye { namespace time {

const uint64_t min = static_cast<uint64_t>(0);
const uint64_t max = std::numeric_limits<uint64_t>::max();

struct time_of_day_t;
struct time_of_day_ms_t;

bool parse_time_of_day(time_of_day_t& lhs, const char* begin);
bool parse_time_of_day_ms(time_of_day_ms_t& lhs, const char* begin);

struct time_of_day_t
{
    time_of_day_t()
    : i(0)
    {}

    explicit time_of_day_t(uint64_t const& nanos)
    : padding{0}
    {
        tm t;
        time_t secs = to_seconds(nanos);
        gmtime::gmtime_r(&secs, &t);
        seconds = t.tm_sec;
        minutes = t.tm_min;
        hours = t.tm_hour;
    }

    explicit time_of_day_t(std::string const& str)
    : i(0)
    {
        parse_time_of_day(*this, str.c_str());
    }

    time_of_day_t(int h, int m, int s)
    : seconds(s)
    , minutes(m)
    , hours(h)
    , padding{0}
    {
        ASSERT(hours < 24);
        ASSERT(minutes < 60);
        ASSERT(seconds < 60);
    }

    time_of_day_t(tm const& rhs)
        : seconds(rhs.tm_sec)
        , minutes(rhs.tm_min)
        , hours(rhs.tm_hour)
        , padding{0}
    {}

    uint64_t nanos() const
    {
        return time::hours(hours) + time::minutes(minutes) + time::seconds(seconds);
    }

    bool operator==(time_of_day_t const& rhs) const { return i == rhs.i; }
    bool operator!=(time_of_day_t const& rhs) const { return i != rhs.i; }
    bool operator>(time_of_day_t const& rhs) const { return i > rhs.i; }
    bool operator>=(time_of_day_t const& rhs) const { return i >= rhs.i; }
    bool operator<(time_of_day_t const& rhs) const { return i < rhs.i; }
    bool operator<=(time_of_day_t const& rhs) const { return i <= rhs.i; }

    union
    {
        int32_t i;
        struct
        {
            int8_t seconds;
            int8_t minutes;
            int8_t hours;
            int8_t padding;
        };
    };
};

inline std::ostringstream& operator<<(std::ostringstream& os, time_of_day_t const& t)
{
    os << std::setfill('0') << std::setw(2) << int(t.hours) << ':'<< std::setfill('0') << std::setw(2) << int(t.minutes) << ':' << std::setw(2) << int(t.seconds);
    return os;
}

inline std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, time_of_day_t const& t)
{
    os << std::setfill('0') << std::setw(2) << int(t.hours) << ':'<< std::setfill('0') << std::setw(2) << int(t.minutes) << ':' << std::setw(2) << int(t.seconds);
    return os;
}



template <typename Emitter>
void json_emit(Emitter* lhs, const time::time_of_day_t& rhs)
{
    std::ostringstream oss;
    oss << rhs;
    lhs->emit(oss.str());
}


template<typename IStream>
inline void json_restore(int node, time::time_of_day_t& lhs, parsing::json_reader<IStream>* reader)
{
    std::string str;
    reader->restore(node, str);
    parse_time_of_day(lhs, str.c_str());
}


inline time_of_day_t parse_time_of_day(std::istringstream & iss)
{
    std::string t;
    iss >> t;
    return time_of_day_t(t);
}


struct time_of_day_ms_t
{
    time_of_day_ms_t() : i(0)
    {
        static_assert(sizeof(*this) == 8, "Expected size");
    }

    explicit time_of_day_ms_t(uint64_t const& nanos)
    : padding0{}
    , padding1{}
    {
        tm t;
        time_t secs = to_seconds(nanos);
        gmtime::gmtime_r(&secs, &t);
        seconds = t.tm_sec;
        minutes = t.tm_min;
        hours = t.tm_hour;
        millis = time::millis(nanos -time::hours(hours) + time::minutes(minutes) + time::seconds(seconds));
    }

    explicit time_of_day_ms_t(std::string const& str)
    : i(0)
    {
        parse_time_of_day_ms(*this, str.c_str());
    }

    time_of_day_ms_t(int h, int m, int s, int ms)
        : padding0(0), millis(ms), seconds(s), minutes(m), hours(h), padding1(0)
    {
        ASSERT(hours < 24);
        ASSERT(minutes < 60);
        ASSERT(seconds < 60);
        ASSERT(millis < 1000);
        ASSERT(padding0 == 0);
        ASSERT(padding1 == 0);

    }

    uint64_t nanos() const
    {
        return time::hours(hours) + time::minutes(minutes) + time::seconds(seconds) + time::millis(millis);
    }

    bool operator==(time_of_day_ms_t const& rhs) const { return i == rhs.i; }
    bool operator!=(time_of_day_ms_t const& rhs) const { return i != rhs.i; }
    bool operator>(time_of_day_ms_t const& rhs) const { return i > rhs.i; }
    bool operator>=(time_of_day_ms_t const& rhs) const { return i >= rhs.i; }
    bool operator<(time_of_day_ms_t const& rhs) const { return i < rhs.i; }
    bool operator<=(time_of_day_ms_t const& rhs) const { return i <= rhs.i; }

    union
    {
        int64_t i;
        struct
        {
            int16_t padding0;
            int16_t millis;
            int8_t  seconds;
            int8_t  minutes;
            int8_t  hours;
            int8_t  padding1;
        };
    };

};

inline std::ostringstream& operator<<(std::ostringstream& os, time_of_day_ms_t const& t)
{
    os << std::setfill('0') << std::setw(2) << int(t.hours) << ':'<< std::setfill('0') << std::setw(2) << int(t.minutes) << ':' << std::setw(2) << int(t.seconds) << '.' << std::setw(3) << int(t.millis);
    return os;
}

inline std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, time_of_day_ms_t const& t)
{
    os << std::setfill('0') << std::setw(2) << int(t.hours) << ':'<< std::setfill('0') << std::setw(2) << int(t.minutes) << ':' << std::setw(2) << int(t.seconds) << '.' << std::setw(3) << int(t.millis);
    return os;
}


template <typename Emitter>
void json_emit(Emitter* lhs, const time::time_of_day_ms_t& rhs)
{
    std::ostringstream oss;
    oss << rhs;
    lhs->emit(oss.str());
}


template<typename IStream>
inline void json_restore(int node, time::time_of_day_ms_t& lhs, parsing::json_reader<IStream>* reader)
{
    std::string str;
    reader->restore(node, str);
    parse_time_of_day_ms(lhs, str.c_str());
}

inline bool parse_time_of_day(time_of_day_t& lhs, const char* begin)
{
    int h, m, s;
    if (sscanf(begin, "%d:%d:%d", &h, &m, &s) != 3)
        return false;
    lhs = time_of_day_t(h, m, s);
    return true;
}


inline bool parse_time_of_day_ms(time_of_day_ms_t& lhs, const char* begin)
{
    int h, m, s, ms;
    if (sscanf(begin, "%d:%d:%d.%d", &h, &m, &s, &ms) != 4)
        return false;
    lhs = time_of_day_ms_t(h, m, s, ms);
    return true;
}



}}

