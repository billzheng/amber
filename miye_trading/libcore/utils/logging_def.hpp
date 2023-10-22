/*
 * logging_def.hpp
 *
 * Purpose: logging enums
 *
 * Author: 
 */

#pragma once

#include "libcore/types/types.hpp"
#include <string.h>

namespace miye
{
namespace utils
{

#define LOGBUFSIZE 512
#define LOGTEXTSIZE (LOGBUFSIZE - 32)

template <typename E>
struct enum_traits;

enum class log_level_t : uint8_t
{
    undefined = 0,
    fatal = 1,
    error = 2,
    warning = 3,
    ui_info = 4,
    info = 5,
    debug = 6,
    trace = 7
};

template <>
struct enum_traits<log_level_t>
{
    static constexpr log_level_t LAST = log_level_t::trace;
    static constexpr log_level_t FIRST = log_level_t::undefined;
    static constexpr bool iterable = true;
};

inline const char* c_str(log_level_t level)
{
    switch (level)
    {
    case log_level_t::undefined:
        return "UNDEFINED";
    case log_level_t::fatal:
        return "FATAL";
    case log_level_t::error:
        return "ERROR";
    case log_level_t::warning:
        return "WARNING";
    case log_level_t::ui_info:
        return "UI_INFO";
    case log_level_t::info:
        return "INFO";
    case log_level_t::debug:
        return "DEBUG";
    case log_level_t::trace:
        return "TRACE";
    }
    return ENUM_UNKNOWN;
}
template <typename T>
inline T from_str(const char* s);

template <>
inline log_level_t from_str<log_level_t>(const char* s)
{
    if (!strncmp("FATAL", s, 5))
        return log_level_t::fatal;
    else if (!strncmp("ERROR", s, 5))
        return log_level_t::error;
    else if (!strncmp("WARNING", s, 7))
        return log_level_t::warning;
    else if (!strncmp("UI_INFO", s, 7))
        return log_level_t::ui_info;
    else if (!strncmp("INFO", s, 4))
        return log_level_t::info;
    else if (!strncmp("DEBUG", s, 5))
        return log_level_t::debug;
    else if (!strncmp("TRACE", s, 5))
        return log_level_t::trace;
    return log_level_t::undefined;
}

template <typename OStream>
OStream& operator<<(OStream& os, log_level_t r)
{
    os << c_str(r);
    return os;
}

//-----------------------------------------------------------------------

enum class log_ex_source_t : uint8_t
{
    undefined = 0,
    process = 1,
    strategy = 2,
    strategy_manager = 3,
    order = 4,
    admin = 5
};

template <>
struct enum_traits<log_ex_source_t>
{
    static constexpr log_ex_source_t LAST = log_ex_source_t::admin;
    static constexpr log_ex_source_t FIRST = log_ex_source_t::undefined;
    static constexpr bool iterable = true;
};

inline const char* c_str(log_ex_source_t source)
{
    switch (source)
    {
    case log_ex_source_t::undefined:
        return "undefined";
    case log_ex_source_t::process:
        return "process";
    case log_ex_source_t::strategy:
        return "strategy";
    case log_ex_source_t::order:
        return "order";
    case log_ex_source_t::strategy_manager:
        return "strategy_manager";
    case log_ex_source_t::admin:
        return "admin";
    }
    return ENUM_UNKNOWN;
}

template <>
inline log_ex_source_t from_str<log_ex_source_t>(const char* s)
{
    if (!strncmp("process", s, 7))
        return log_ex_source_t::process;
    else if (!strncmp("strategy_manager", s, 16))
        return log_ex_source_t::strategy_manager;
    else if (!strncmp("strategy", s, 8))
        return log_ex_source_t::strategy;
    else if (!strncmp("order", s, 5))
        return log_ex_source_t::order;
    else if (!strncmp("admin", s, 5))
        return log_ex_source_t::admin;
    return log_ex_source_t::undefined;
}

template <typename OStream>
OStream& operator<<(OStream& os, log_ex_source_t r)
{
    os << c_str(r);
    return os;
}

} // namespace utils
} // namespace miye
