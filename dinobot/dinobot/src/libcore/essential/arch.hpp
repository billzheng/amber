/*
 * arch.hpp
 * Purpose: architecture specific routines
 */

#pragma once
#include <stdint.h>

namespace dinobot {
  namespace essential {

#if defined(__x86_64__)
// read the time stamp counter

#if !defined(__tune_haswell__)
    inline uint64_t rdtscp() {
      uint64_t lo, hi;
      __asm__ volatile("cpuid\n\t"
                       "rdtsc\n\t"
                       : "=r"(hi), "=r"(lo));
      return ((uint64_t)lo) | (((uint64_t)hi) << 32);
    }

#else

    inline uint64_t rdtscp() {
      uint64_t hi, lo;
      __asm__ volatile("rdtscp" : "=a"(lo), "=d"(hi) : : "%rcx");

      return (uint64_t)lo | (((uint64_t)hi) << 32);
    }
#endif // RDTSC

#else //__x86_64__
#error "x86-64 only"
#endif
  }
}
