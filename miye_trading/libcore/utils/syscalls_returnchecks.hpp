/*
 * syscalls_returnchecks.hpp
 * Purpose: used by syscalls_*.hpp to check return values
 * and throw exceptions/abort if not kosher
 * Templated on whether exceptions are thrown
 * Author: 
 */

#pragma once

#include "git_version.h"
#include "libcore/essential/backtrace.hpp"
#include "libcore/essential/exception.hpp"
#include <iostream>

namespace miye
{
namespace syscalls
{

static inline void dump_backtrace(void) noexcept
{
    std::cerr << "git version: " << GIT_VERSION << std::endl;
    auto bt = backtrace::backtrace();
    for (auto& a : bt)
    {
        std::cerr << a << std::endl;
    }
}

#ifdef USE_EXCEPTIONS_IN_CHECKS
template <typename T, typename T2>
inline void check_notval(T ret, T2 val, const char* caller)
{
    if (ret == val)
    {
        throw essential::exception(essential::exception::syscall_error)
            << "syscall_error " << ::strerror(errno) << " from: " << caller;
    }
}

inline void check_not_neg(int ret, const char* caller,
                          const char* moreinfo = "")
{
    if (ret < 0)
    {
        throw essential::exception(essential::exception::syscall_error)
            << "syscall_error " << ::strerror(errno) << " " << moreinfo << " "
            << " from: " << caller;
    }
}

static inline void check_nonnull(void* ret, const char* caller)
{
    check_notval(ret, nullptr, caller);
}

static inline void check_not_eof(int ret, const char* caller)
{
    check_notval(ret, EOF, caller);
}

#else

template <typename T, typename T2>
inline void check_notval(T ret, T2 val, const char* caller) noexcept(true)
{
    if (ret == val)
    {
        std::cerr << "syscall_error " << ::strerror(errno)
                  << " from: " << caller << std::endl;
        dump_backtrace();
        ::abort();
    }
}

inline void check_not_neg(int ret, const char* caller,
                          const char* moreinfo = "") noexcept(true)
{
    if (ret < 0)
    {
        std::cerr << "syscall error " << ::strerror(errno) << " " << moreinfo
                  << " "
                  << " from: " << caller << std::endl;
        dump_backtrace();
        ::abort();
    }
}

static inline void check_nonnull(void* ret, const char* caller) noexcept(true)
{
    check_notval(ret, nullptr, caller);
}

static inline void check_not_eof(int ret, const char* caller) noexcept(true)
{
    check_notval(ret, EOF, caller);
}

#endif // USE_EXCEPTIONS_IN_CHECKS

} // namespace syscalls
} // namespace miye
