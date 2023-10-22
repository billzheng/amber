/*
 * gzfile.hpp
 * Purpose: read a gzipped mmfile - archival analysis
 * not latency critical
 *
 *
 */
#pragma once

#include "zlib.h"

#include "libcore/essential/assert.hpp"
#include "libcore/essential/exception.hpp"
#include "mmap_headers.hpp"
#include "mmap_reader.hpp"
#include "qstream_reader_interface.hpp"

//#LINKFLAGS=-lz

namespace miye
{
namespace qstream
{

static const int chunksize = 2 * 64 * 1024;

struct unzipper
{
    unzipper()
        : fd_(-1), filepos(0), bytes_last_unzip{0}, total_unzipped{0},
          prebuffered{0}, complete_{false}, last_unzip_(false), read_size(0)
    {
        stream.zalloc = Z_NULL;
        stream.zfree = Z_NULL;
        stream.opaque = Z_NULL;
        stream.avail_in = 0;
        stream.next_in = Z_NULL;
        int ret = inflateInit2(&stream, 16 + MAX_WBITS);
        INVARIANT(ret == Z_OK);
    }

    void set_fd(int fd)
    {
        fd_ = fd;
    }

    int unzip_n_bytes_peek(int n)
    {
        auto ret = unzip_n_bytes(n);
        prebuffered = n;
        return ret;
    }
    int unzip_n_bytes(int n, size_t offset = 0)
    {
        INVARIANT_MSG(n <= chunksize,
                      "can't  get more than "
                          << chunksize << " bytes at a time, asked for: " << n);
        bytes_last_unzip = 0;
        int zipped_bytes = syscalls::read<false>(fd_, in_buf, chunksize);
        if (zipped_bytes == EOF)
        {
            complete_ = true;
            bytes_last_unzip = 0;
            return Z_OK;
        }
        stream.next_in = in_buf;

        stream.avail_in = zipped_bytes;

        stream.avail_out = n - prebuffered - offset;
        stream.next_out = out_buf + prebuffered + offset;
        int ret = inflate(&stream, Z_NO_FLUSH);
        INVARIANT(ret != Z_STREAM_ERROR);
        if (ret == Z_NEED_DICT)
        {
            std::cerr << "error is " << ret << std::endl;
            ret = Z_DATA_ERROR;
        }
        bytes_last_unzip = n - stream.avail_out;
        total_unzipped += bytes_last_unzip;
        if (ret == Z_DATA_ERROR || ret == Z_MEM_ERROR)
        {
            std::cerr << "error is " << ret << std::endl;
            (void)inflateEnd(&stream);
            complete_ = true;
            return ret;
        }

        if (ret == Z_STREAM_END)
        {
            (void)inflateEnd(&stream);
            last_unzip_ = true;
        }
        rewind_n_bytes(stream.avail_in);
        prebuffered = 0;
        return Z_OK;
    }

    uint64_t current_pos()
    {
        return lseek(fd_, 0, SEEK_CUR);
    }

    void rewind_n_bytes(int n)
    {
        filepos = current_pos();
        filepos -= n;
        filepos = lseek(fd_, filepos, SEEK_SET);
    }

    unsigned char* unzipped_bytes()
    {
        return out_buf;
    }

    size_t unzipped_size()
    {
        return bytes_last_unzip;
    }

    void reset()
    {
        lseek(fd_, 0, SEEK_SET);
        total_unzipped = 0;
    }

    bool done()
    {
        return complete_;
    }

    bool last_unzip()
    {
        return last_unzip_;
    }

    uint64_t unzipped_file_offset()
    {
        return total_unzipped;
    }

    unsigned char* peek(size_t size)
    {
        return read(size, true);
    }

    unsigned char* read(size_t size, bool peek = false)
    {
        INVARIANT_MSG(size < chunksize,
                      "Requested more than buffer size:" << size << "\n");

        if (size > bytes_last_unzip - read_size)
        {
            if (last_unzip_)
            {
                complete_ = true;
                return nullptr;
            }
            memmove(out_buf, out_buf + read_size, bytes_last_unzip - read_size);
            unzip_n_bytes(chunksize - (bytes_last_unzip - read_size),
                          (bytes_last_unzip - read_size));
            read_size = 0;
        }
        auto loc = out_buf + read_size;
        if (!peek)
        {
            read_size += size;
        }
        return loc;
    }

    int fd_;
    uint64_t filepos;
    int bytes_last_unzip;
    uint64_t total_unzipped;
    uint64_t prebuffered;
    bool complete_;
    bool last_unzip_;
    z_stream stream;

    unsigned char out_buf[chunksize + 1];
    unsigned char in_buf[chunksize + 1];

    // internal buffer cursor
    size_t read_size;
};

template <typename Clock>
class gzfile : public qstream_reader_interface<gzfile<Clock>>
{
  public:
    gzfile(Clock& clk_, const std::string& description_)
        : clk(clk_), description(description_), rec_size(0)
    {
        initialize();
    }
    ~gzfile()
    {
    }

    void initialize()
    {
        int flags = O_RDONLY;

        streamoptions options = extract_streamoptions(description);
        timed = (static_cast<streamoptions>(streamoption::timed) & options);
        auto path = extract_path(description);
        fd_ = syscalls::open<false>(path.c_str(), flags, 0644);
        uzip.set_fd(this->fd_);
        uzip.reset();

        unsigned char* p = uzip.read(sizeof(*fileheader));
        fileheader = reinterpret_cast<mmap_header*>(p);

        INVARIANT_MSG(chunksize == uzip.unzipped_size() || uzip.done() ||
                          uzip.last_unzip(),
                      DUMP(chunksize)
                          << DUMP(uzip.unzipped_size()) << DUMP(uzip.done()));

        INVARIANT_MSG(fileheader->magic == mmap_magic,
                      " bad mmap header " << std::hex << DUMP(fileheader->magic)
                                          << " " DUMP(mmap_magic));

        rec_size = extract_recordsize(description);
        if (rec_size == 0)
        {
            rec_size = fileheader->rec_size;
        }
        next_ts = *reinterpret_cast<uint64_t*>(uzip.peek(sizeof(next_ts)));

        INVARIANT_MSG(rec_size == fileheader->rec_size,
                      "record size check mismatch provided option vs in file"
                          << DUMP(rec_size) << DUMP(fileheader->rec_size));
    }

    const place read(bool fast_forwarding = false)
    {
        place descriptor;
        uint64_t timestamp;
        size_t read_size;
        size_t this_rec_size;
        void* p;

        if (uzip.done())
        {
            return place::eof();
        }

        if (!rec_size)
        {
            p = uzip.read(sizeof(record_header<true>));
            if (uzip.done())
            {
                return place::eof();
            }
            record_header<true>* r = reinterpret_cast<record_header<true>*>(p);
            timestamp = r->timestamp;
            this_rec_size = r->rec_size;
            // already read the header, calculate num remaining bytes to read
            read_size =
                ROUND_UP(this_rec_size + sizeof(*r), this->entry_alignment) -
                sizeof(*r);
            p = uzip.read(read_size);
        }
        else
        {
            this_rec_size = rec_size;
            read_size = ROUND_UP(rec_size + sizeof(record_header<false>),
                                 this->entry_alignment);
            p = uzip.read(read_size);
            if (uzip.done())
            {
                return place::eof();
            }
            record_header<false>* r =
                reinterpret_cast<record_header<false>*>(p);
            timestamp = r->timestamp;
            p = r->payload;
        }

        if (timed && !fast_forwarding)
        {
            clk.set(timestamp);
        }
        return place(p, this_rec_size);
    }

    void slow_attest(uint64_t* next_ts)
    {
        attest(next_ts);
    }

    void attest(uint64_t* next_timestamp)
    {
        auto ts_loc = reinterpret_cast<uint64_t*>(uzip.peek(sizeof(next_ts)));
        if (uzip.done())
        {
            // ready to return eof
            if (*next_timestamp != withdrawn_timestamp)
            {
                *next_timestamp = clk.now();
            }
            return;
        }
        next_ts = *ts_loc;
        *next_timestamp = next_ts;
    }

    void fast_forward(uint64_t stop_ts = 0)
    {
        if (!stop_ts)
        {
            stop_ts = last_write_ts();
        }
        auto cur_stream_ts =
            *reinterpret_cast<uint64_t*>(uzip.peek(sizeof(next_ts)));
        while (cur_stream_ts < stop_ts)
        {
            auto plc = read(true);
            if (plc.is_eof())
            {
                break;
            }
            cur_stream_ts =
                *reinterpret_cast<uint64_t*>(uzip.peek(sizeof(next_ts)));
        }
    }

    uint64_t last_write_ts()
    {
        return fileheader->write_timestamp;
    }

    Clock& clk;
    const std::string description;

  private:
    static const uint64_t entry_alignment = CACHE_LINE_SIZE;
    uint64_t rec_size;
    int fd_;
    mmap_header* fileheader;
    uint64_t status;
    bool timed;
    uint64_t next_ts;
    unzipper uzip;
};

} // namespace qstream
} // namespace miye
