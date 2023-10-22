/*
 * arch_defs.hpp
 *
 * Purpose: defines useful for target x86-64 avx2 eanabled cpus
 */

#include "libcore/essential/assert.hpp"

#pragma once

#define CACHE_LINE_SIZE 64

#define TRUNCATE(num, almnt)     ((num) & (-(almnt)))
#define ROUND_UP(num, almnt)     (((num) + ((almnt) - 1)) & ~((almnt) - 1))
#define ALIGN(addr)        __attribute__((aligned(addr)))
#define ALIGNED(num, almnt)   (!!((num) & ~((almnt) - 1)))

#define PAGE_SIZE        (4 * 1024)
#define SUPERPAGE_SIZE   (2 * 1024 * 1024)
//
// AVX, AVX2
#define VECTOR_ALIGNMENT (32)

//wrap some gcc builtins so code compiles on non gnuc compilers
#if defined(__GNUC__)
#define RESTRICT __restrict
#define COLD __attribute((cold))
#define HOT __attribute((hot))
#define LIKELY(expr)    __builtin_expect(!!(expr), 1)
#define UNLIKELY(expr)  __builtin_expect(!!(expr), 0)
#define PACKED __attribute__((packed))
#define ALWAYS_INLINE inline __attribute__((__always_inline__))
#define NEVER_INLINE __attribute__((__noinline__))
#define PREFETCH_READ_NT(addr) __builtin_prefetch(addr, 0, 0)
#define PREFETCH_READ(addr) __builtin_prefetch(addr, 0, 3)
#define PREFETCH_WRITE(addr) __builtin_prefetch(addr, 1, 3)
#else
#define RESTRICT
#define COLD
#define HOT
#define LIKELY
#define UNLIKELY
#define PACKED
#define ALWAYS_INLINE inline
#define NEVER_INLINE
#define PREFETCH_READ
#define PREFETCH_WRITE
#define PREFETCH_READ_NT
#endif


#define UNUSED(n)   { (void)n; }

#define ASSUME_ALIGNED(type, addr, almnt) (static_cast<type *>(__builtin_assume_aligned(static_cast<const void *>(addr), (almnt))))
#define ASSERT_ALIGNED(ptr, almnt)                                    \
    do { ASSERT(((uintptr_t)ptr & ((almnt) - 1)) == 0); } while (0)
#define INVARIANT_ALIGNED(ptr, almnt)                                 \
    do { INVARIANT(((uintptr_t)ptr & ((almnt) - 1)) == 0); } while (0)
#define INVARIANT_ALIGNED_MSG(ptr, almnt, msg)                         \
    do { INVARIANT_MSG(((uintptr_t)ptr & ((almnt) - 1)) == 0, msg); } while (0)

