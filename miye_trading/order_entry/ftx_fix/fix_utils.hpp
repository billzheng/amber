#pragma once
#include "libcore/time/gmtime.hpp"
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <time.h>

namespace miye::trading::fix
{
inline size_t get_single_fix_msg_len(size_t body_len)
{
    // 8=FIX.4.2|9=194|  <-- 16
    // 10=107|           <-- 7

    size_t fix_msg_len{body_len};

    if (body_len >= 100)
    {
        fix_msg_len += 23;
    }
    else
    {
        fix_msg_len += 22;
    }

    return fix_msg_len;
}

inline size_t get_fix_body_len(const char* p)
{
    size_t fix_body_len = 0;
    const char* begin(p + 12);
    while (*begin != '\001')
    {
        fix_body_len *= 10;
        fix_body_len += *begin - '0';
        ++begin;
    }
    return fix_body_len;
}
//
///*
// * TODO: refactr to use date.h
// */
// inline std::string get_last_sunday()
//{
//    auto tt = time(0);
//
//    tt /= 60 * 60 * 24;
//    tt *= 60 * 60 * 24;
//
//    auto tm = gmtime::gmtime(&tt);
//
//    while (tm->tm_wday != 0)
//    {
//        tt -= 60 * 60 * 24;
//        tm = gmtime::gmtime(&tt);
//    }
//
//    std::stringstream ss;
//    auto lt = gmtime::gmtime(&tt);
//    ss << std::put_time(lt, "%Y%m%d");
//    return ss.str();
//}
///*
// * TODO: refactor to use date.h
// */
// inline std::string get_today()
//{
//    auto tt = time(0);
//
//    tt /= 60 * 60 * 24;
//    tt *= 60 * 60 * 24;
//
//    auto tm = gmtime::gmtime(&tt);
//    std::stringstream ss;
//
//    ss << std::put_time(tm, "%Y%m%d");
//    return ss.str();
//}

} // namespace miye::trading::fix
