/*
 * date.hpp
 *
 * Purpose: date
 *
 * Author: 
 */
#pragma once

#include <string>

#include "../math/math_utils.hpp"
#include "libcore/parsing/json.hpp"
#include "libcore/time/timeutils.hpp"
#include "gmtime.hpp"


namespace miye { namespace time {

static const int DAYS_BEFORE_MONTH[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
// offset 0 is 2012
static const int JAN_1_DOW[26] = {
                      /* 2015 */  weekday_t::thursday, weekday_t::friday, weekday_t::sunday,
                      /* 2018 */  weekday_t::monday, weekday_t::tuesday, weekday_t::wednesday,
                      /* 2021 */  weekday_t::friday, weekday_t::saturday, weekday_t::sunday,
                      /* 2024 */  weekday_t::monday, weekday_t::wednesday, weekday_t::thursday,
                      /* 2027 */  weekday_t::friday, weekday_t::saturday, weekday_t::monday,
                      /* 2030 */  weekday_t::tuesday, weekday_t::wednesday, weekday_t::thursday,
                      /* 2033 */  weekday_t::saturday, weekday_t::sunday, weekday_t::monday,
                      /* 2036 */  weekday_t::tuesday, weekday_t::thursday, weekday_t::friday,
                      /* 2039 */  weekday_t::saturday, weekday_t::sunday
                      };
static const int IS_LEAP[26] = {0, //2015
                                1, 0, 0, 0, 1, 0, 0, 0,
                                1, 0, 0, 0, 1, 0, 0, 0,
                                1, 0, 0, 0, 1, 0, 0, 0,
                                1  //2040 - add more if needed
                                };

static const int START_YEAR = 2015;


inline int days_in_month(int month, bool leap_year)
{
    static int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (leap_year && month == 1)
        return 29;
    ASSERT(month < 12 && month >= 0);
    return days_in_month[month];
}


struct date_t;

bool parse_date(date_t& lhs, const char* begin);
date_t date_from_int(uint32_t d);


struct date_t
{
    date_t()
    : i(0)
    {}

    explicit date_t(std::string const& str)
    : i(0)
    {
        parse_date(*this, str.c_str());
    }

    explicit date_t(uint64_t now)
    {
        tm t;
        time_t secs = to_seconds(now);
        gmtime::gmtime_r(&secs, &t);
        day = t.tm_mday;
        month = t.tm_mon+1;
        year = t.tm_year+1900;
    }


    explicit date_t(int y, int m, int d)
        : day(d), month(m), year(y)
    {
        ASSERT(d < 32);
        ASSERT(m < 13);
    }

   explicit  date_t(tm const& rhs)
        : day(rhs.tm_mday)
        , month(rhs.tm_mon+1)
        , year(rhs.tm_year+1900) {}


    bool is_quarterly() const
    {
        return month == 3 || month == 6 || month == 9 || month == 12;
    }

    bool is_weekend() const
    {
        tm t_m;
        populate_tm(t_m);
        return t_m.tm_wday == 0 || t_m.tm_wday == 6;
    }

    bool is_leap_year() const {
        ASSERT(year > 1582);
        if (year % 400 == 0)
           return true;
        else if (year % 100 == 0)
           return false;
        else if (year % 4 == 0)
           return true;
        return false;
    }

    date_t next_day() const
    {
        int8_t next_day = day + 1;
        int8_t next_month = month;
        int16_t next_year = year;
        if (next_day > days_in_month(month-1, is_leap_year()))
        {
            next_day = 1;
            next_month = month + 1;
            if (next_month > 12)
            {
                next_month = 1;
                next_year = year + 1;
            }
        }
        return date_t(next_year, next_month, next_day);
    }


    date_t prev_day() const
    {
        int8_t prev_day = day - 1;
        int8_t prev_month = month;
        int16_t prev_year = year;
        if (prev_day == 0)
        {
            prev_month = month - 1;
            if (prev_month == 0)
            {
                prev_month = 12;
                prev_year = year - 1;
            }
            prev_day = days_in_month(prev_month-1, is_leap_year());
        }
        return date_t(prev_year, prev_month, prev_day);
    }

    int days_diff(date_t other_date, size_t max_diff, bool ignore_weekends) const
    {
        int d = 0;
        if (*this > other_date)
        {
            while(*this != other_date)
            {
                other_date = other_date.next_day();
               if(math::fast_abs(d) > max_diff){
                   break;
               }
               if (!ignore_weekends || (ignore_weekends && ! other_date.is_weekend()))
                   --d;
            }
        }
        else if (*this < other_date)
        {
            while(*this != other_date)
            {
                other_date = other_date.prev_day();
                if(d > int(max_diff)){
                    break;
                }
                if (!ignore_weekends || (ignore_weekends && ! other_date.is_weekend()))
                    ++d;
            }
        }
        return d;
    }

    union
    {
        int32_t i;
        struct
        {
            int8_t day;        // 1..31
            int8_t month;      // 1..12
            int16_t year;      // eg 2017
        };
    };

    int32_t to_yyyymmdd() const
    {
        return day + month * 100 + year * 10000;
    }

    bool operator==(date_t const& rhs) const { return i == rhs.i; }
    bool operator!=(date_t const& rhs) const { return i != rhs.i; }
    bool operator>(date_t const& rhs) const { return i > rhs.i; }
    bool operator>=(date_t const& rhs) const { return i >= rhs.i; }
    bool operator<(date_t const& rhs) const { return i < rhs.i; }
    bool operator<=(date_t const& rhs) const { return i <= rhs.i; }

    void populate_tm(tm& rhs) const
    {
        memset(&rhs, 0, sizeof(tm));
        ASSERT(year >= 1900);
        rhs.tm_mday = day;
        rhs.tm_mon = month - 1;
        rhs.tm_year = year - 1900;
        rhs.tm_hour = rhs.tm_min = rhs.tm_sec = 0;
        rhs.tm_wday = calc_wday();
        rhs.tm_yday = calc_yday();
    }
    int calc_wday() const
    {
        INVARIANT_MSG(year >= 2015, "we don't calculate the weekday for years 2015 and before");
        int yr_idx = year - START_YEAR;
        int days_since_year_start = DAYS_BEFORE_MONTH[month - 1] + day -1;
        ((month - 1) > month_t::februrary) && IS_LEAP[yr_idx] && ++days_since_year_start;
        int wday = (days_since_year_start + JAN_1_DOW[yr_idx]) % 7;
        return wday;
    }

    int calc_yday() const
    {
        return DAYS_BEFORE_MONTH[month - 1] + day -1;
    }
};


template<typename OStream>
OStream& operator<<(OStream& os, date_t const& d)
{
    if(d.i > 0)
        os << std::setfill('0') << std::setw(4) << d.year << std::setw(2) << int(d.month) << std::setw(2) << int(d.day);
    else
        os << "0";
    return os;
}

template <typename Emitter>
void json_emit(Emitter* lhs, const time::date_t& rhs)
{
    std::ostringstream oss;
    oss << rhs;
    lhs->emit(oss.str());
}

template<typename IStream>
inline void json_restore(int node, time::date_t& lhs, parsing::json_reader<IStream>* reader)
{
    if (reader->get_node(node).type != parsing::json_type_t::string) {
        INVARIANT_FAIL("Json error restoring date: expected a string");
    }
    std::string str;
    reader->restore(node, str);
    if (!str.empty() && std::stoi(str) > 0 && !parse_date(lhs, str.c_str())) {
        INVARIANT_FAIL("Json error restoring date: invalid format: " << str);
    }

}

date_t date_from_epoch_days(uint32_t days);

}}
