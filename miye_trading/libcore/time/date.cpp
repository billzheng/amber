/*
 * date.hpp
 *
 * Purpose: date
 *
 * Author: 
 */

#include "libcore/time/date.hpp"

#include <string>

namespace miye { namespace time {
bool parse_date(date_t& lhs, const char* begin)
{
    tm t;
    memset(&t, 0, sizeof(tm));
    const char* s = strptime(begin, "%Y%m%d", &t);
    if (s == nullptr)
        return false;
    if (s - begin != 8)
        return false;
    lhs = date_t(t);
    return true;
}

date_t date_from_int(uint32_t d)
{
    uint32_t year = (d / 10000);
    uint32_t month = (d - ((d/10000)*10000))/100;
    uint32_t day = d - (year * 10000) - (month * 100);
    return date_t(year, month, day);

}

date_t date_from_epoch_days(uint32_t days)
{
    uint64_t nanos = days * time::hours(24);
    return date_t(nanos - 1);
}

}}

