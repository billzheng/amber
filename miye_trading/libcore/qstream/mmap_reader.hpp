/*
 * mmap_reader.hpp
 * purpose: read a structured mmap
 * Author: 
 */

#pragma once

#include "arbiter_common.hpp"
#include "libcore/essential/assert.hpp"
#include "libcore/essential/platform_defs.hpp"
#include "libcore/time/clock.hpp"
#include "libcore/utils/syscalls_files.hpp"
#include "libcore/utils/syscalls_mmap.hpp"
#include "mmap_headers.hpp"
#include "qstream_common.hpp"
#include "qstream_reader_interface.hpp"

#include <unistd.h>

namespace miye
{
namespace qstream
{

template <typename Clock>
class alignas(CACHE_LINE_SIZE) mmap_reader
    : public qstream_reader_interface<mmap_reader<Clock>>
{
    typedef qstream_reader_interface<mmap_reader<Clock>> parent_t;

  public:
    mmap_reader(const mmap_reader&) = delete;
    mmap_reader& operator=(const mmap_reader&) = delete;
    mmap_reader(mmap_reader&& other)
        : read_offset(other.read_offset), current_record(other.current_record),
          recordsize(other.recordsize), file_descriptor(other.file_descriptor),
          header(other.header), mapping_size(other.mapping_size),
          clock(other.clock), options(other.options), membase(other.membase),
          file_offset(other.file_offset), description(other.description),
          moved(false)
    {
        other.moved = true;
        other.read_offset = 0;
        other.current_record = nullptr;
        other.recordsize = -1;
        other.file_descriptor = -1;
        other.header = nullptr;
        other.mapping_size = 0;
        other.membase = nullptr;
        other.file_offset = 0;
    }
    COLD mmap_reader(Clock& clock_, std::string description_)
        : parent_t(), read_offset(0), current_record(nullptr), recordsize(0),
          file_descriptor(0), header(nullptr),
          mapping_size(default_mapped_size), clock(clock_), file_offset(0),
          description(description_), moved(false)
    {
        // FIXME - uncomment ad test alignment requirement!
        // INVARIANT_ALIGNED(this, CACHE_LINE_SIZE);
        std::string path = extract_path(description);
        options = extract_streamoptions(description);
        mapping_size = extract_mappingsize(description, default_mapped_size);
        recordsize = extract_recordsize(description);
        if (is_wait(options))
        {
            // spin until it's there
            wait(path);
        }
        bool anticipate = is_anticipate(description);
        if (anticipate)
        {
            // anticipate implies follows
            options |= static_cast<streamoptions>(streamoption::follow);
            if (access(path.c_str(), F_OK) == -1)
            {
                // mmfile not there yet so we'll anticipate it
                // just create a header with a write offset of
                // no data written. can go in an arbiter, attest will return
                // time::max until the writer shows up and writes something
                // to it.
                file_descriptor =
                    syscalls::open(path.c_str(), O_RDWR | O_CREAT, 0644);
                syscalls::ftruncate64(file_descriptor, min_mmap_size);
                mmap_header* dummy_header = reinterpret_cast<mmap_header*>(
                    syscalls::mmap64(0,
                                     min_mmap_size,
                                     PROT_WRITE | PROT_READ,
                                     MAP_SHARED,
                                     file_descriptor,
                                     0));
                ::memset(dummy_header, 0, min_mmap_size);
                // set magic number and recordsize to valid values
                dummy_header->magic = mmap_magic;
                dummy_header->rec_size = recordsize;
                // indicate no write at all past the header
                dummy_header->write_offset = sizeof(mmap_header);
                dummy_header->write_timestamp = 0;
                syscalls::msync(
                    dummy_header, min_mmap_size, MS_INVALIDATE | MS_SYNC);
                ::munmap(dummy_header, sizeof(mmap_header));
                ::close(file_descriptor);
            }
        }

        file_descriptor = syscalls::open(path.c_str(), O_RDONLY);

        // check file size is bigger than sizeof(mmap_header)

        header = reinterpret_cast<mmap_header*>(syscalls::mmap64(
            0, min_mmap_size, PROT_READ, MAP_SHARED, file_descriptor, 0));
        INVARIANT_MSG(mmap_magic == header->magic,
                      "file had magic=" << std::hex << header->magic
                                        << " we were expecting=" << mmap_magic
                                        << " for a qstream mmap. qstream "
                                        << description);

#if 0
        INVARIANT_MSG(recordsize == static_cast<int>(header->rec_size),
                      "reader started with wrong recordsize option given: " << DUMP(recordsize) << " != " << DUMP(header->rec_size));
#endif

        read_offset = sizeof(*header);
        membase = reinterpret_cast<char*>(syscalls::mmap64(
            0, mapping_size, PROT_READ, MAP_SHARED, file_descriptor, 0));

        current_record = membase + sizeof(*header);
    }
    HOT const place read() noexcept
    {

        // unless we're following this file
        // if our read_offset has caught up to the header write_offset we're
        // done, return eof could make this branchless
        auto mapping_end = membase + mapping_size;
        if (UNLIKELY((!follows(options)) &&
                     (read_offset >= header->write_offset)))
        {
            if (UNLIKELY(current_record + mmap_max_recordsize > mapping_end))
            {
                extend_mapping();
            }
            return place::eof();
        }

        ASSERT_MSG(read_offset <= header->write_offset,
                   DUMP(read_offset) << DUMP(header->write_offset));
        uint64_t mask = -(!(recordsize));
        // mask all 1s when recordsize is variable sizeof(record_header<false>)
        // == 8, <true> == 16
        uint64_t rec_header_size = 8 ^ ((8 ^ 16) & mask);
        size_t var_size = reinterpret_cast<volatile const record_header<true>*>(
                              current_record)
                              ->rec_size;
        size_t size = recordsize ^ ((var_size ^ recordsize) & mask);
        // timestamp at same offset regardless of fixed size or variable.
        uint64_t read_timestamp =
            reinterpret_cast<volatile const record_header<false>*>(
                current_record)
                ->timestamp;
        size_t bytes_used = size + rec_header_size;
        char* payload = const_cast<char*>(current_record) + rec_header_size;

        if (UNLIKELY(current_record + mmap_max_recordsize > mapping_end))
        {
            extend_mapping();
        }

        current_record += ROUND_UP(bytes_used, CACHE_LINE_SIZE);

        ASSERT_MSG(current_record + ROUND_UP(bytes_used, CACHE_LINE_SIZE) <
                       membase + mapping_size,
                   DUMP(description)
                       << std::hex << DUMP((void*)current_record)
                       << DUMP((void*)membase) << DUMP(mapping_size)
                       << DUMP((void*)mapping_end) << DUMP(bytes_used));
        read_offset += ROUND_UP(bytes_used, CACHE_LINE_SIZE);

        // do we need to set the clock?
        if (is_timed(options) && clock.can_set())
        {
            clock.set(read_timestamp);
        }

        return place(payload, size);
    }

    COLD void slow_attest(uint64_t* next_ts) noexcept
    {
        if (read_offset >= header->write_offset)
        {
            if (follows(options))
            {
                *next_ts = nothing_to_read;
            }
            else if (*next_ts != withdrawn_timestamp)
            {
                *next_ts = clock.now();
            }
            return;
        }
        auto r = reinterpret_cast<const volatile record_header<true>*>(
            current_record);
        auto ts = r->timestamp;
        *next_ts = ts;
    }
    HOT void attest(uint64_t* next_ts) noexcept
    {

        // Invariant: current record must be inside the currently mapped region
        ASSERT_MSG(current_record > membase &&
                       current_record < membase + mapping_size,
                   "current record must be inside mapping"
                       << DUMP((void*)current_record) << DUMP((void*)membase)
                       << DUMP(mapping_size));

        // timestamp at same offset whether variable or fixed record so no need
        // to check which

/* **************************************************************************************

   the commented out implementation is left as an explanation for the branchless
   version
*/
#if 0
        if(read_offset >= header->write_offset) {
            if(!follows(options)) {
                // hit eof
                *next_ts = clock.now();
            } else {
                *next_ts = nothing_to_read;
            }
            return;
        }
        auto r = reinterpret_cast<const volatile record_header<true>*>(current_record);
        auto ts = r->timestamp;
        *next_ts = ts;

 /*
 ******************************************************************************************  */
#else

        // the same as the above, branchless - about 30ns faster in benchmarking
        // requires both the reader and writer map ahead of the current record
        // to the header of the next store memory fence - worth 20-60 cycles to
        // average ping pong
        asm volatile("sfence" ::: "memory");
        auto r = reinterpret_cast<volatile const record_header<true>*>(
            current_record);
        auto ts = r->timestamp;

        auto prev = *next_ts;

        // calculate whether we would return now() for eof or nothing_to_read
        // if we have read to the end of the stream
        auto now = clock.now();
        uint64_t endval_mask = -(!follows(options));
        auto end_val =
            nothing_to_read ^ ((now ^ nothing_to_read) & endval_mask);

        // end_val now contains the correct thing if we've read to the end

        // choose between end_val and r->timestamp based on if we have read to
        // end
        uint64_t at_end_mask = -(!(read_offset < header->write_offset));
        uint64_t retval = ((ts) ^ ((ts ^ end_val) & at_end_mask));
        // all 1s if we're already withdrawn, don't update *next_ts
        uint64_t withdrawn_mask = (-!(prev != withdrawn_timestamp));
        retval = retval ^ ((retval ^ prev) & withdrawn_mask);
        *next_ts = retval;
#endif
    }

    COLD void fast_forward(uint64_t stop_ts = 0)
    {
        if (!stop_ts)
        {
            stop_ts = last_write_ts();
        }
        auto mapping_end = membase + mapping_size;
        while (read_offset < header->write_offset)
        {
            size_t bytes_used = 0;
            if (!recordsize)
            {
                bytes_used =
                    reinterpret_cast<volatile const record_header<true>*>(
                        current_record)
                        ->rec_size +
                    sizeof(record_header<true>);
            }
            else
            {
                bytes_used = recordsize + sizeof(record_header<false>);
            }
            uint64_t read_timestamp =
                reinterpret_cast<volatile const record_header<false>*>(
                    current_record)
                    ->timestamp;
            if (read_timestamp > stop_ts)
            {
                break;
            }
            if (UNLIKELY(current_record + mmap_max_recordsize > mapping_end))
            {
                extend_mapping();
            }
            current_record += ROUND_UP(bytes_used, CACHE_LINE_SIZE);
            read_offset += ROUND_UP(bytes_used, CACHE_LINE_SIZE);
        }
    }

    COLD uint64_t last_write_ts()
    {
        return header->write_timestamp;
    }

    COLD ~mmap_reader()
    {
        // std::cerr << __func__ << "() called\n";
        if (!moved)
        {
            if (file_descriptor != -1)
            {
                ::flock(file_descriptor, LOCK_UN);
                ::close(file_descriptor);
            }
            if (header)
            {
                ::munmap((void*)header, min_mmap_size);
            }
            if (membase)
            {
                ::munmap((void*)membase, mapping_size);
            }
        }
    }

  private:
    COLD void extend_mapping() noexcept
    {
        // std::cerr << "reader::" << __func__ << "() " << std::hex <<
        // DUMP(file_offset) << std::endl;
        file_offset += mapping_size / 2;
        membase = static_cast<char*>(syscalls::mmap64(membase,
                                                      mapping_size,
                                                      PROT_READ,
                                                      MAP_SHARED,
                                                      file_descriptor,
                                                      file_offset));
        current_record = membase + read_offset - file_offset;
    }

    COLD void wait(std::string path)
    {
        // Log something here to say we're going to wait for it?
        // do we want this to timeout and fail?
        std::cerr << "waiting for " << path << " to exist" << std::endl;
        while (access(path.c_str(), F_OK) == -1)
        {
            ::usleep(100000);
            ; // just spin
        }
        ::usleep(1000000);
        // Log something here to say we got it?
        std::cerr << path << " is now there!" << std::endl;
    }

    // ******** first cacheline                 //byte offset

    // mmap reader specific
    uint64_t read_offset;                // 0
    volatile const char* current_record; // 8

    // common to mmap
    int recordsize;      // 24
    int file_descriptor; // 28
    // without the volatile a branchless arbiter::ruling
    // will optimise attest() away to nop
    volatile const mmap_header* header; // 32
    size_t mapping_size;                // 40

    // common to qstream
    Clock& clock;          // 48
    streamoptions options; // 56

    // cacheline boundary here

    // ******** second cache line - less frequently used

    // mmap reader specific
    char* membase; // 64 only used to actually perform a remapping, take the
                   // cache hit then
    uint64_t file_offset; // 72 only used to perfrom remapping

    // common to mmap

    // common to qstream
  public:
    std::string description; // 80
    bool moved;

} ALIGN(CACHE_LINE_SIZE);

} // namespace qstream
} // namespace miye
