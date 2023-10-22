#ifndef LIB_RING_COMMON_H
#define LIB_RING_COMMON_H

#include <cstdint>
#include <cstddef>
#include <sys/mman.h>


#ifndef likely
#define likely(x)       __builtin_expect((x),1)
#endif

#ifndef unlikely
#define unlikely(x)     __builtin_expect((x),0)
#endif

namespace dinobot { namespace lib { namespace shm {

struct ring_header
{
    uint32_t ring_magic;
    uint32_t version;
    uint32_t element_size;
    uint32_t total_elements;
    uint32_t num_readers;
    uint32_t writer_sequence;

    char padding[44];
};

struct ring_reader_record
{
    uint32_t sequence;
    uint32_t pid;

    char padding[56];
};

struct ring_element_header
{
    volatile uint32_t sequence;
    uint32_t size;
    volatile uint64_t writer_timestamp;
    volatile uint64_t chunks;
};

struct ring_reader_retval
{
	const char * buffer;
	uint32_t size;
    uint64_t writer_timestamp;
    uint64_t chunks;
};

enum
{
    // version number of the file format implemented in this version of ring
    ring_version_number = 0x01,

    // magic number to identify the file
    ring_magic = 0x33474E52,

    // start of the reader array
    ring_reader_offset = 0x40,

    // size of each entry in the reader array
    ring_reader_record_size = 0x40,

    // start of ring elements
    ring_element_offset = 0x1000,

    // mapping size required for mapping just the header
    ring_header_pagesize = 0x1000,

    // x86 provides large, 2MB pages. Our rings are, therefore, multiples of 2MB
	// MAYBE change this if we go to 1gb hugepages...
    ring_pagesize = 0x200000,
};

inline size_t ring_buffer_size(size_t element_size, size_t total_elements)
{
    size_t size = element_size * total_elements + ring_element_offset;

    // round size to next multiple of ring_pagesize
    return (size + (ring_pagesize - 1)) & ~(ring_pagesize - 1);
}

/**
 * Calculates the integer log base 2 of the given value.
 *
 * N.B.  int_log2(0) returns 0.
 */
inline uint32_t int_log2(uint64_t x)
{
    uint64_t res;
    __asm__("bsr %1, %0" : "=r"(res) : "0"(x));
    return (uint32_t)res;
}

/**
 * If *ptr == o, set *ptr = n.  Otherwise return *ptr.
 */
static inline int32_t cmpxchg32(volatile uint32_t* ptr, uint32_t o, uint32_t n)
{
    asm volatile
    (
        "lock cmpxchg %2, %0"
        : "=m"(*ptr), "=&a"(o), "=&r"(n)
        : "m"(*ptr), "1"(o), "2"(n)
        : "cc"
    );
    return o;
}


} } }

#endif /* RING_COMMON_H */
