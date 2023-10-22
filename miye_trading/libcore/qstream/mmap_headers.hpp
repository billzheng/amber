/*
 * mmap_headers.hpp
 * Purpose: define the mmap file headers and record headers
 */
#pragma once

#include "libcore/essential/platform_defs.hpp"

namespace miye
{
namespace qstream
{

#define SIZE_64k (64 * 1024)
// 0x786572746e617571 is "quantrex" as a little endian 64bit int
static const uint64_t mmap_magic = 0x786572746e617571; // 8675466101592782193
static const uint32_t min_mmap_size = SUPERPAGE_SIZE;
static const uint32_t mmap_max_recordsize = min_mmap_size;
static const uint32_t default_mapped_size = mmap_max_recordsize * 8;

struct mmap_header
{
    uint64_t magic;
    uint64_t rec_size;
    uint64_t write_offset;
    uint64_t write_timestamp;
    // pad out to 64*1024 bytes, can use it for store of info about the stream
    char unstructured[SIZE_64k - 4 * sizeof(uint64_t)];
};
static_assert(sizeof(mmap_header) == SIZE_64k, "header size");

// note zero length arrays below
// https://gcc.gnu.org/onlinedocs/gcc/Zero-Length.html

template <bool Variable>
struct record_header
{
    uint64_t timestamp;
    uint64_t rec_size;
    char payload[0];
};

template <>
struct record_header<false>
{
    uint64_t timestamp;
    char payload[0];
};

} // namespace qstream
} // namespace miye
