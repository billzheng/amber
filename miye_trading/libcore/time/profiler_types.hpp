/*
 * profiler_types.hpp
 *
 * Purpose: profiler enums
 *
 * Author: 
 */

#pragma once
#include "libcore/types/types.hpp"
#include <string.h>
#include <stdint.h>

namespace miye { namespace time {

// add new values at the end to remain compatible

enum class profiler_trigger: uint32_t {
    none = 0,
    generic,
    engine_fast_path,
    engine_read,
    engine_processing,
    engine_signal_time,
    engine_traded_time,
    engine_send_time,
    om_fix_msg_prep,
    om_msg_sent

};

inline const char* c_str(profiler_trigger t)
{
    switch(t)
    {
    case profiler_trigger::none:                return "none";
    case profiler_trigger::generic:             return "generic";
    case profiler_trigger::engine_fast_path:    return "engine_fast_path";
    case profiler_trigger::engine_read:         return "engine_read";
    case profiler_trigger::engine_processing:   return  "engine_processing";
    case profiler_trigger::engine_signal_time:  return  "engine_signal_time";
    case profiler_trigger::engine_traded_time:  return  "engine_traded_time";
    case profiler_trigger::engine_send_time:    return  "engine_send_time";
    case profiler_trigger::om_fix_msg_prep:     return  "om_fix_msg_prep";
    case profiler_trigger::om_msg_sent:         return  "om_msg_sent";
    }
    return ENUM_UNKNOWN;
}

template<typename OStream>
OStream& operator<<(OStream& os, profiler_trigger t)
{
    switch(t)
    {
    case profiler_trigger::none:
        os << "none";
        break;
    case profiler_trigger::generic:
        os << "generic";
        break;
    case profiler_trigger::engine_fast_path:
        os << "engine_fast_path";
        break;
    case profiler_trigger::engine_read:
        os << "engine_read";
        break;
    case profiler_trigger::engine_processing:
        os << "engine_processing";
        break;
    case profiler_trigger::engine_signal_time:
        os << "engine_signal_time";
        break;
    case profiler_trigger::engine_traded_time:
        os << "engine_traded_time";
        break;
    case profiler_trigger::engine_send_time:
        os << "engine_send_time";
        break;
    case profiler_trigger::om_fix_msg_prep:
        os << "om_fix_msg_prep";
        break;
    case profiler_trigger::om_msg_sent:
        os << "om_msg_sent";
        break;
    }
    return os;
}

template <typename T>
inline T from_str(const char* s);

template<>
inline profiler_trigger from_str<profiler_trigger>(const char* s)
{
    if (!::strncmp("generic", s, strlen(s))) {
        return profiler_trigger::generic;
    } else if (!::strncmp("engine_fast_path", s, strlen(s))) {
        return profiler_trigger::engine_fast_path;
    } else if (!::strncmp("engine_read", s, strlen(s))) {
        return profiler_trigger::engine_read;
    } else if (!::strncmp("engine_processing", s, strlen(s))) {
        return profiler_trigger::engine_processing;
    } else if (!::strncmp("engine_signal_time", s, strlen(s))) {
        return profiler_trigger::engine_signal_time;
    } else if (!::strncmp("engine_traded_time", s, strlen(s))) {
        return profiler_trigger::engine_traded_time;
    } else if (!::strncmp("engine_send_time", s, strlen(s))) {
        return profiler_trigger::engine_send_time;
    } else if (!::strncmp("om_fix_msg_prep", s, strlen(s))) {
        return profiler_trigger::om_fix_msg_prep;
    } else if (!::strncmp("om_msg_sent", s, strlen(s))) {
        return profiler_trigger::om_msg_sent;
    }
    return profiler_trigger::none;
}


}}
