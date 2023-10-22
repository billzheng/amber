/*
 * mmap_writer.hpp
 * purpose: write a structured mmap
 * Author: 
 */

#pragma once

#include "libcore/essential/platform_defs.hpp"
#include "libcore/utils/syscalls_files.hpp"
#include "libcore/utils/syscalls_libc.hpp"
#include "libcore/utils/syscalls_mmap.hpp"
#include "mmap_headers.hpp"
#include "qstream_common.hpp"
#include "qstream_writer_interface.hpp"

namespace miye
{
namespace qstream
{

template <typename Clock>
class alignas(CACHE_LINE_SIZE) mmap_writer
    : public qstream_writer_interface<mmap_writer<Clock>>
{
    typedef qstream_writer_interface<mmap_writer<Clock>> parent_t;

  public:
    mmap_writer(const mmap_writer&) = delete;
    mmap_writer& operator=(const mmap_writer&) = delete;
    mmap_writer(mmap_writer&& other) noexcept
        : file_base_offset(other.file_base_offset), file_size(other.file_size),
          next_record(other.next_record), write_offset(other.write_offset),
          recordsize(other.recordsize), file_descriptor(other.file_descriptor),
          header(other.header), mapping_size(other.mapping_size),
          clock(other.clock), membase(other.membase),
          description(other.description),
          initial_filesize(other.initial_filesize), moved(false)
    {
        other.moved = true;
        // set the moved from object members to invalid
        other.file_base_offset = 0;
        other.file_size = 0;
        other.next_record = nullptr;
        other.write_offset = 0;
        other.recordsize = -1;
        other.file_descriptor = -1;
        other.header = nullptr;
        other.mapping_size = 0;
        other.membase = nullptr;
        other.initial_filesize = 0;
    }
    COLD mmap_writer(Clock& clock_, std::string description_)
        : file_base_offset(0), next_record(0), header(nullptr), clock(clock_),
          membase(nullptr), description(description_), moved(false)
    {
        INVARIANT_ALIGNED_MSG(this, CACHE_LINE_SIZE, description_);
        bool created_mmap = false;
        // extract path from description
        std::string path = extract_path(description);

        streamoptions options = extract_streamoptions(description);
        recordsize = extract_recordsize(description);
        mapping_size = extract_mappingsize(description,
                                           default_mapped_size); // default 16M
        initial_filesize = extract_initial_filesize(
            description, file_increase_delta());                 // default 16M
        mapping_size = std::max(mapping_size, initial_filesize); // default 16M
        INVARIANT_MSG(mapping_size > min_mmap_size,
                      "mapping_size must be bigger than min_mmap_size "
                          << DUMP(mapping_size) << DUMP(min_mmap_size));

        file_descriptor = syscalls::open(path.c_str(), O_RDWR | O_CREAT, 0644);
        syscalls::flock(file_descriptor, LOCK_EX | LOCK_NB);

        struct stat64 statinfo;
        syscalls::fstat64(file_descriptor, &statinfo);
        if (must_create_set(options))
        {
            INVARIANT_MSG(statinfo.st_size == 0,
                          path << " : must_create streamoption set but file "
                                  "already exists with size "
                               << statinfo.st_size);
        }
        if (statinfo.st_size <= min_mmap_size)
        {
            syscalls::ftruncate64(file_descriptor, file_increase_delta());
            file_size = file_increase_delta();
            created_mmap = true;
        }
        else
        {
            // file exists, we're going to append to it
            file_size = statinfo.st_size;
        }

        header = reinterpret_cast<mmap_header*>(
            syscalls::mmap64(0,
                             min_mmap_size,
                             PROT_WRITE | PROT_READ,
                             MAP_SHARED,
                             file_descriptor,
                             0));

        bool maybe_extend = false;
        if (created_mmap)
        {
            syscalls::memset(header, 0, sizeof(mmap_header));
            header->magic = mmap_magic;
            header->rec_size = recordsize;
            write_offset = sizeof(mmap_header);
            header->write_offset = write_offset;
            header->write_timestamp = 0;
            syscalls::msync(header, min_mmap_size, MS_INVALIDATE | MS_SYNC);
        }
        else
        {
            // file exists, we're going to append to it
            write_offset = header->write_offset;
            maybe_extend = true;
            INVARIANT_MSG(recordsize == header->rec_size,
                          "writer started with wrong recordsize option given: "
                              << DUMP(recordsize)
                              << " != " << DUMP(header->rec_size));
        }

        membase =
            reinterpret_cast<char*>(syscalls::mmap64(0,
                                                     mapping_size,
                                                     PROT_READ | PROT_WRITE,
                                                     MAP_SHARED,
                                                     file_descriptor,
                                                     0));

        if (maybe_extend)
        {
            auto mapping_end = file_base_offset + mapping_size;
            while (write_offset >= mapping_end)
            {
                extend_mapping(true);
                mapping_end = file_base_offset + mapping_size;
            }
        }

#ifdef NDEBUG // this fails when running under valgrind
        // touch all the memory to fault it in
        auto extent = std::min(mapping_size, initial_filesize);
        auto end_addr = (uint64_t)membase + extent - 1;
        auto start_addr = membase;
        char* cur = reinterpret_cast<char*>(TRUNCATE(end_addr, PAGE_SIZE));
        while (cur > start_addr)
        {
            volatile uint64_t* nothing = reinterpret_cast<uint64_t*>(cur);
            cur -= PAGE_SIZE;
            INVARIANT_ALIGNED(cur, PAGE_SIZE);
            UNUSED(nothing);
        }
#endif // NDEBUG

        // if we had to extend the mapping, next record is already set
        if (!next_record)
        {
            next_record = membase + (write_offset - file_base_offset);
        }
    }

    HOT place pledge(size_t required_sz) noexcept
    {
        ASSERT(!recordsize || (required_sz <= static_cast<size_t>(recordsize)));

        auto mapping_end = file_base_offset + mapping_size;
        char* start = nullptr;

        uint64_t mask = -(!(recordsize));
        // mask all 1s when recordsize is 0, variable
        uint64_t rec_header_size =
            (sizeof(record_header<false>) ^
             ((sizeof(record_header<false>) ^ sizeof(record_header<true>)) &
              mask));

        size_t full_reqd_sz =
            ROUND_UP(required_sz + 2 * rec_header_size, CACHE_LINE_SIZE);

        // check file and mapping are big enough before return -- this record
        // and the following one following record may need to have its timestamp
        // read (and ignored) before actually being written
        auto required_offset = write_offset + full_reqd_sz;

        if (UNLIKELY(required_offset + mmap_max_recordsize > file_size))
        {
            extend_file();
        }
        if (UNLIKELY(required_offset > mapping_end))
        {
            extend_mapping();
        }
#if 0
        if(!recordsize) {
            start = reinterpret_cast<record_header<true>*>(next_record)->payload;
        } else {
            start = reinterpret_cast<record_header<false>*>(next_record)->payload;
        }
#else
        // branchless version of the above
        // mask all 1s when recordsize is variable
        // start of data is at offset 16 for variable sized, 8 for fixed.
        // i.e. at end of record header

        start = (next_record + rec_header_size);

#endif
        return place(start, required_sz);
    }

    HOT int announce(const place& pledged)
    {
        auto now = clock.now();
        uint64_t bytes_used;
        // can make this branchless
        if (!recordsize)
        {
            // variable size
            record_header<true>* rec =
                reinterpret_cast<record_header<true>*>(next_record);
            rec->timestamp = now;
            rec->rec_size = pledged.size;
            bytes_used = ROUND_UP(pledged.size + sizeof(*rec), CACHE_LINE_SIZE);
        }
        else
        {
            // fixed
            record_header<false>* rec =
                reinterpret_cast<record_header<false>*>(next_record);
            rec->timestamp = now;
            bytes_used = ROUND_UP(recordsize + sizeof(*rec), CACHE_LINE_SIZE);
        }
        // do we need this header update to be atomic?
        // don't need anything special x86 memory model takes
        // care of it for us "reads may not be reordered with older writes to
        // the same location"
        write_offset += bytes_used;
        next_record += bytes_used;
        header->write_timestamp = now;
        header->write_offset = write_offset;

        return 0; // success
    }

    HOT void warm()
    {
        // prefetching the header and next record cut the minmum ping pong
        // latency seen by about half but without both prefetches the average is
        // 30% better

        PREFETCH_WRITE(header);
        PREFETCH_WRITE(next_record);

        // ping pong approx latency in cycles
        // test with 3m repititions
        // prefetched   min     avg
        // neither     550      675
        // header      520      900
        // next_record 500      750
        // both        300      640

#if 0

        // deeper prefetching appears to have no effect
        //PREFETCH_WRITE(next_record + SUPERPAGE_SIZE);
        //PREFETCH_WRITE(next_record + SUPERPAGE_SIZE + SUPERPAGE_SIZE);

        // this makes it slower in the presence of kernel ticks
        //PREFETCH_WRITE(this);
#endif
    }

    COLD ~mmap_writer()
    {
        // std::cerr << __func__ << "() called\n";
        if (!moved)
        {
            // direct api use - ignore failures
            if (file_descriptor != -1)
            {
                ::flock(file_descriptor, LOCK_UN);
                ::close(file_descriptor);
            }
            if (header)
            {
                ::munmap(header, min_mmap_size);
            }
            if (membase)
            {
                ::munmap(membase, mapping_size);
            }
        }
    }

    Clock& get_clock()
    {
        return clock;
    };

  private:
    //
    COLD uint64_t file_increase_delta()
    {
        return mapping_size;
    }
    void extend_file()
    {
        // std::cerr << "writer::" <<  __func__ << "() " << std::hex <<
        // DUMP(file_size) << DUMP(file_increase_delta()) <<
        // DUMP(file_base_offset) << std::endl;
        file_size += file_increase_delta();
        syscalls::ftruncate64(file_descriptor, file_size);
    }
    COLD void extend_mapping(bool on_startup = false)
    {
        // move only half of the mapped size forward in the file
        // std::cerr << "writer::" << __func__ << "() " << std::hex <<
        // DUMP(file_size) << DUMP(file_increase_delta()) <<
        // DUMP(file_base_offset) << std::endl;
        file_base_offset += mapping_size / 2;
        membase = static_cast<char*>(syscalls::mmap64(membase,
                                                      mapping_size,
                                                      PROT_READ | PROT_WRITE,
                                                      MAP_SHARED,
                                                      file_descriptor,
                                                      file_base_offset));
        uint64_t dist_into_window = 0;
        if ((write_offset - file_base_offset) < mapping_size)
        {
            dist_into_window = write_offset - file_base_offset;
        }
        else
        {
            INVARIANT_MSG(on_startup,
                          DUMP(description) << DUMP(write_offset) << "- "
                                            << DUMP(file_base_offset)
                                            << ">= " << DUMP(mapping_size)
                                            << DUMP(file_size));
        }
        next_record = membase + dist_into_window;
        ASSERT_MSG(next_record < membase + mapping_size,
                   DUMP(next_record) << DUMP(membase) << DUMP(mapping_size));
        ASSERT_MSG(next_record >= membase,
                   DUMP(next_record) << DUMP(membase) << DUMP(mapping_size)
                                     << DUMP(dist_into_window));
    }

    // ******** first cacheline                 //byte offset

    // mmap writer specific
    uint64_t file_base_offset; // 0
    uint64_t file_size;        // 8
    char* next_record;         // 16
    // write_offset shadows header->write_offset
    // so we can update it and write it to the header
    // without having to read the header
    uint64_t write_offset; // 24

    // common to mmap
    uint32_t recordsize; // 32
    int file_descriptor; // 36
    mmap_header* header; // 40
    size_t mapping_size; // 48

    // common to qstream
    Clock& clock; // 56

    // cacheline boundary here

    // ******** second cache line - less frequently used

    // mmap reader specific
    // none

    // common to mmap
    // none
    char* membase; // 64 only used to extend mapping

    // common to qstream
  public:
    std::string description; // 72
  private:
    size_t initial_filesize; // 80
    bool moved;
} ALIGN(CACHE_LINE_SIZE);

} // namespace qstream
} // namespace miye
