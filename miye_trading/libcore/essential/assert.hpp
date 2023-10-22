/*
 * assert.hpp
 *
 * Purpose: invariant verification
 */

#include <stdlib.h>
#include <iostream>

#include "libcore/essential/backtrace.hpp"

#pragma once

#ifndef NDEBUG
#define DEBUG_MODE "on"
#else
#define DEBUG_MODE "off"
#endif

#define DUMP(x) #x << ": " << x << " "
#define DUMP_ARR(x, i) #x << "[" << +i <<  "]" << ": " << x[i] << " "
// assert that checks in release mode


#define CARP(expr, msg) \
    do                                                          \
    {                                                           \
        if (__builtin_expect(!!(!(expr)),0))                    \
        {                                                       \
            std::cerr <<  __FILE__ << ":" << __LINE__           \
                    << ": invariant (" << #expr << ")"          \
                    << " failed. " << std::endl                 \
                    << msg << std::endl                         \
                    << "DEBUG_MODE=" << DEBUG_MODE << std::endl;\
            auto bt = miye::backtrace::backtrace();               \
            for (auto& e: bt)                                   \
                std::cerr << e << std::endl;                    \
        }                                                       \
    }                                                           \
    while (0)



#define INVARIANT_MSG(expr, msg)                                \
    do                                                          \
    {                                                           \
        if (__builtin_expect(!!(!(expr)),0))                    \
        {                                                       \
            std::cerr <<  __FILE__ << ":" << __LINE__           \
                    << ": invariant (" << #expr << ")"          \
                    << " failed. " << std::endl                 \
                    << msg << std::endl                         \
                    << "DEBUG_MODE=" << DEBUG_MODE << std::endl;\
            auto bt = miye::backtrace::backtrace();               \
            for (auto& e: bt)                                   \
                std::cerr << e << std::endl;                    \
            ::abort();                                          \
        }                                                       \
    }                                                           \
    while (0)

#define INVARIANT(expr) INVARIANT_MSG(expr, "")
#define INVARIANT_FAIL(msg) INVARIANT_MSG(false, msg)

#define UNIMPLEMENTED(...)  INVARIANT_FAIL(__func__ << "() " << __FILE__ << ":" << __LINE__ << "Unimplemented." << __VA_ARGS__)

// traditional assert that only exists in debug compiles
#ifndef NDEBUG
#define ASSERT_MSG(expr, msg)  INVARIANT_MSG(expr, msg)
#define ASSERT(expr) ASSERT_MSG(expr, "")
#define ASSERT_FAIL(msg) ASSERT_MSG(fail, msg)
#else
#define ASSERT_MSG(expr, msg)    do {} while (0)
#define ASSERT(expr)    do {} while (0)
#define ASSERT_FAIL(expr, msg)    do {} while (0)
#endif  // NDEBUG

#define SAME_TRUTH(a, b) (!((!a) ^ (!b)))



