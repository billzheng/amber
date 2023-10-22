
/*
 * pcap_writer.hpp
 * purpose: write a libpcap compilant capture file
 * Author: 
 */

#include "libcore/time/timeutils.hpp"
#include "mmap_writer.hpp"
#include "pcap_headers.hpp"
#include "qstream_writer_interface.hpp"

#pragma once

namespace miye
{
namespace qstream
{

template <typename Clock>
class pcap_writer : public qstream_writer_interface<pcap_writer<Clock>>
{
    pcap_writer(const pcap_writer&) = delete;
    pcap_writer& operator=(const pcap_writer&) = delete;

  public:
    pcap_writer(Clock& clock_, std::string description_)
        : file_base_offset(0), next_record(0), clock(clock_), membase(nullptr),
          description(description_), moved(false)
    {
        // extract path from description
        std::string path = extract_path(description);
        streamoptions options = extract_streamoptions(description);

        // todo get snaplen (streamoption?)
        uint32_t snaplen = 0x40000;
        qstream::pcap_hdr_t default_hdr = prime_pcap_hdr(snaplen);

        file_descriptor = syscalls::open(path.c_str(), O_RDWR | O_CREAT, 0644);
        syscalls::flock(file_descriptor, LOCK_EX | LOCK_NB);
        struct stat64 statinfo;
        syscalls::fstat64(file_descriptor, &statinfo);

        bool append_mode = false;

        if (must_create_set(options))
        {
            INVARIANT_MSG(statinfo.st_size == 0,
                          path << " : must_create streamoption set but file "
                                  "already exists with size "
                               << statinfo.st_size);
        }
        if (statinfo.st_size <= default_mapped_size)
        {
            syscalls::ftruncate64(file_descriptor, default_mapped_size);
            file_size = default_mapped_size;
        }
        else
        {
            // file exists, we're going to append to it
            file_size = statinfo.st_size;
            append_mode = true;
        }

        membase =
            reinterpret_cast<char*>(syscalls::mmap64(0,
                                                     default_mapped_size,
                                                     PROT_READ | PROT_WRITE,
                                                     MAP_SHARED,
                                                     file_descriptor,
                                                     0));

        if (append_mode)
        {
            // check the header is valid
            qstream::pcap_hdr_t* file_hdr =
                reinterpret_cast<qstream::pcap_hdr_t*>(membase);
            bool header_ok =
                (file_hdr->magic_number == default_hdr.magic_number) &&
                (file_hdr->snaplen == default_hdr.snaplen) &&
                (file_hdr->network == default_hdr.network);
            INVARIANT_MSG(header_ok, DUMP(*file_hdr));
            auto mapping_end = file_base_offset + default_mapped_size;
            while (write_offset >= mapping_end)
            {
                extend_mapping(true);
                mapping_end = file_base_offset + default_mapped_size;
            }
        }
        else
        {
            // write the header
            memcpy(membase, &default_hdr, sizeof(default_hdr));
            write_offset = sizeof(default_hdr);
        }
        if (!next_record)
        {
            next_record = membase + (write_offset - file_base_offset);
        }
    }

    place pledge(size_t required_sz) noexcept
    {

        auto mapping_end = file_base_offset + default_mapped_size;
        char* start = nullptr;

        size_t full_reqd_sz = required_sz + sizeof(pcaprec_hdr_t);

        // check file and mapping are big enough before return -- this record
        // and the following one following record may need to have its timestamp
        // read (and ignored) before actually being written
        auto required_offset = write_offset + full_reqd_sz;

        if (UNLIKELY(required_offset > file_size))
        {
            extend_file();
        }
        if (UNLIKELY(required_offset > mapping_end))
        {
            extend_mapping();
        }
        start = next_record + sizeof(pcaprec_hdr_t);
        return place(start, required_sz);
    }

    int announce(const place& pledged)
    {
        auto now = clock.now();
        uint64_t bytes_used = pledged.size + sizeof(pcaprec_hdr_t);
        // variable size
        pcaprec_hdr_t* rec = reinterpret_cast<pcaprec_hdr_t*>(next_record);
        rec->ts_sec = time::to_seconds(now);
        // we store the nanoseconds part in ts_usec
        rec->ts_usec = now - (time::seconds(time::to_seconds(now)));
        rec->incl_len = pledged.size;
        rec->orig_len = pledged.size;

        write_offset += bytes_used;
        next_record += bytes_used;
        return 0; // success
    }

  private:
    void extend_file()
    {
        file_size += default_mapped_size;
        syscalls::ftruncate64(file_descriptor, file_size);
    }
    void extend_mapping(bool on_startup = false)
    {
        file_base_offset += default_mapped_size / 2;
        membase = static_cast<char*>(syscalls::mmap64(membase,
                                                      default_mapped_size,
                                                      PROT_READ | PROT_WRITE,
                                                      MAP_SHARED,
                                                      file_descriptor,
                                                      file_base_offset));
        uint64_t dist_into_window = 0;

        if ((write_offset - file_base_offset) < default_mapped_size)
        {
            dist_into_window = write_offset - file_base_offset;
        }
        else
        {
            INVARIANT(on_startup);
        }
        next_record = membase + dist_into_window;
        ASSERT_MSG(next_record < membase + default_mapped_size,
                   DUMP(next_record)
                       << DUMP(membase) << DUMP(default_mapped_size));
        ASSERT_MSG(next_record >= membase,
                   DUMP(next_record)
                       << DUMP(membase) << DUMP(default_mapped_size)
                       << DUMP(dist_into_window));
    }

    // ******** first cacheline                 //byte offset

    // pcap writer specific
    uint64_t file_base_offset; // 0
    uint64_t file_size;        // 8
    char* next_record;         // 16
    uint64_t write_offset;     // 24
    int file_descriptor;       // 32
    char pad[4];               // 36

    // common to qstream
    Clock& clock;  // 40
    char* membase; // 48

    // cacheline boundary here

    // ******** second cache line - less frequently used

    // none

    // common to qstream
  public:
    std::string description; // 56
  private:
    bool moved;
};

} // namespace qstream
} // namespace miye
