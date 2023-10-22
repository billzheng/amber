#ifndef RING_WRITER_H
#define RING_WRITER_H

#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/file.h>
#include <exception>
#include <csignal>

#include "ring_common.h"
#include "sys_utils.h"

namespace dinobot { namespace lib { namespace shm {

/**
 * A writer to a single-write, multi-reader ring buffer.
 */
struct ring_writer
{

    ring_writer(std::string filename, uint32_t size, uint32_t elems, uint32_t readers )
        : buffer_(NULL),
          current_write_(NULL),
          shutdown_(false),
          buffer_size_(0),
          fd_(-1)
    {
        filename_ = filename;
        uint32_t requested_element_size = size;
        uint32_t requested_total_elements = elems;

        num_readers_ = readers;

        // reader records must fit into the first 4k page
        if ((ring_reader_offset + num_readers_ * ring_reader_record_size)
                > ring_header_pagesize)
            throw std::runtime_error("too many readers");

        // first eight bytes are for storing how long the packet is
        element_size_ = requested_element_size + sizeof(uint64_t);

        // round element_size to the next highest power of two
        element_size_log2_ = int_log2(element_size_ - 1) + 1;
        element_size_ = 1 << element_size_log2_;

        // round total_elements to the next highest power of two
        total_elements_ = 1 << (int_log2(requested_total_elements - 1) + 1);

        // open the file and set up the mappings
		// this also sets buffer_ 
        open_ring();

        // starting sequence number
        next_sequence_ = ((ring_header*)buffer_)->writer_sequence;
        update_cached_reader_sequence();

        // read from, and then write back to, the start of each element
        // to ensure it is pulled into the TLB
        for (size_t i = 0; i < total_elements_; i++)
        {
            size_t offset = ring_element_offset + (i << element_size_log2_);
            volatile char c = *(volatile char*)(buffer_ + offset);
            *(volatile char*)(buffer_ + offset) = c;
        }
    }

    ~ring_writer()
    {
        close_ring();
    }

    //bool write(const char *data, size_t len,  bool cond)
    /* chunks are 
     *   0 : single packet / or last in chunks
     *   x : x more packets in this chunk
     */
    // TODO ensure chunks stuff works... 
    bool write(const char *data, size_t len, uint64_t rec_ts, uint64_t chunks)
    {
        char *r = begin_write(len, rec_ts, chunks);
        ::memcpy(r, data, len);
        return end_write();
    }


    char * begin_write(size_t size, uint64_t rec_ts, uint64_t chunks)
    {
        if (unlikely((size + sizeof(uint64_t)) > (1U << element_size_log2_)))
            throw std::runtime_error("write larger than mtu");


        // block until there is space in the ring
        if (unlikely((next_sequence_ - cached_reader_sequence_)
                    >= total_elements_))
            wait_for_readers();

        current_write_ = buffer_ + ring_element_offset + ((next_sequence_ & (total_elements_ - 1)) << element_size_log2_);

        ((ring_element_header*)current_write_)->size = size;
        ((ring_element_header*)current_write_)->writer_timestamp = rec_ts;
        ((ring_element_header*)current_write_)->chunks = chunks;

        return current_write_ + sizeof(ring_element_header);
    }

    bool end_write()
    {
        if (unlikely(shutdown_))
            return false;

        asm volatile("sfence");

        // this commits the changes and makes it readable to any readers
        ((ring_element_header*)current_write_)->sequence = next_sequence_;
        next_sequence_++;

        // update sequence - only used for when the writer needs to restart
        ((ring_header*)buffer_)->writer_sequence = next_sequence_;

        return true;
    }

    /*
    bool end_write_resize(size_t size, bool cond)
    {
        if (unlikely((size + sizeof(ring_element_header)) >
                    (1U << element_size_log2_)))
            throw std::runtime_error("write larger than mtu");
        ((ring_element_header*)current_write_)->size = size;
        return end_write(cond);
    }
	*/

    void shutdown_write()
    {
        shutdown_ = true;
    }

    size_t mtu() const
    {
        return element_size_ - sizeof(ring_element_header);
    }

private:
    /**
     * Block until readers make progress and there is space in the ring.
     */
	// TODO maybe can leave this if we don't want to block on readers??
	// either that of jsut increase the size of the buffer... 
    void wait_for_readers()
    {
        update_cached_reader_sequence();
        if ((next_sequence_ - cached_reader_sequence_) < total_elements_)
            return;

        // we have to block - output warnings if we are waiting on a
        // nonexistent process
        for (uint32_t i = 0; i < num_readers_; i++)
        {
            ring_reader_record* r = (ring_reader_record*)(buffer_ +
                    ring_reader_offset + i * ring_reader_record_size);
            if (r->sequence == cached_reader_sequence_)
            {
                if (r->pid == 0 || (kill(r->pid, 0) == -1 && errno == ESRCH))
					std::cout << ": waiting on id " << i << " which has no reader" << std::endl;
            }
        }

        while ((next_sequence_ - cached_reader_sequence_) >= total_elements_)
            update_cached_reader_sequence();
    }

    /**
     * Set cached_reader_sequence_ to the minimum of all reader sequence
     * numbers.
     */
    void update_cached_reader_sequence()
    {
        cached_reader_sequence_ = next_sequence_;
        for (uint32_t i = 0; i < num_readers_; i++)
        {
            ring_reader_record* r = (ring_reader_record*)(buffer_ +
                    ring_reader_offset + i * ring_reader_record_size);
            if (next_sequence_ - r->sequence > next_sequence_ -
                    cached_reader_sequence_)
                cached_reader_sequence_ = r->sequence;
        }
    }

    /**
     * Open a ring with the given filename, element size, etc,
     * and set up the memory mapping for the ring.
     * Throws on error.
     */
    void open_ring()
    {
        if (fd_ != -1)
            ::close(fd_);

        // Open and lock the file - allow for blocking if shared is true
        fd_ = base::atomic_open_for_write(filename_, false, true, false);

        buffer_size_ = ring_buffer_size(element_size_, total_elements_);

        struct stat s;
        ::fstat(fd_, &s);

        if (s.st_size != 0)
        {
            // Check that the header parameters match
            ring_header* header = (ring_header*)::mmap(NULL,
                    ring_header_pagesize, PROT_READ | PROT_WRITE,
                    MAP_SHARED, fd_, 0);
            if (header == MAP_FAILED)
            {
                close(fd_);
                fd_ = -1;
				throw std::runtime_error("mmap failed");
            }
            ring_header header_copy = *header;
            ::munmap(header, ring_header_pagesize);

            if (header_copy.ring_magic != ring_magic ||
                    header_copy.version != ring_version_number ||
                    header_copy.element_size != element_size_ ||
                    header_copy.total_elements != total_elements_ ||
                    header_copy.num_readers != num_readers_)
            {
				std::stringstream ss;
				ss  << "invalid format:" << 
                    " magic=" << header_copy.ring_magic <<
                    " (expected " << (int)ring_magic << ")" <<
                    " version=" << header_copy.version <<
                    " (expected " << (int)ring_version_number << ")" <<
                    " element_size=" << header_copy.element_size <<
                    " (expected " << element_size_ << ")" <<
                    " total_elements=" << header_copy.total_elements <<
                    " (expected " << total_elements_ << ")" <<
                    " num_readers=" << header_copy.num_readers <<
                    " (expected " << num_readers_ << ")";
				throw std::runtime_error(ss.str());
            }
        }

        // If it is a new file, set up the header
        if (s.st_size == 0)
        {
            if (::ftruncate(fd_, buffer_size_))
            {
                ::close(fd_);
				throw std::runtime_error("ftruncate failed");
            }

            ring_header* header = (ring_header*)::mmap(NULL,
                    ring_header_pagesize, PROT_READ | PROT_WRITE, MAP_SHARED,
                    fd_, 0);
            if (header == MAP_FAILED)
            {
                close(fd_);
                fd_ = -1;
				throw std::runtime_error("mmap failed");
            }

            header->version = ring_version_number;
            header->element_size = element_size_;
            header->total_elements = total_elements_;
            header->writer_sequence = total_elements_;
            header->num_readers = num_readers_;

            for (uint32_t i = 0; i < num_readers_; i++)
                ((ring_reader_record*)((char*)header + ring_reader_offset +
                    i * ring_reader_record_size))->sequence = total_elements_;

            // The last thing to do is write the magic,
            // which effectively enables the readers to read it.
            header->ring_magic = ring_magic;

            ::munmap(header, ring_header_pagesize);
        }

        // Finally, set up the mapping which we will use
        buffer_ = (char*)::mmap(NULL, buffer_size_, PROT_READ | PROT_WRITE,
                MAP_SHARED, fd_, 0);
        if (buffer_ == MAP_FAILED)
        {
            ::close(fd_);
            fd_ = -1;
            buffer_ = NULL;
            throw std::runtime_error("mmap failed");
        }
    }

    /**
     * Remove the memory mappings and close the file.
     */
    void close_ring()
    {
        if (buffer_ != NULL)
        {
            if (::munmap(buffer_, buffer_size_) == -1)
				throw std::runtime_error("munmap failed");
            buffer_ = NULL;
            buffer_size_ = 0;
        }
        if (fd_ != -1)
        {
            ::flock(fd_, LOCK_UN);
            ::close(fd_);
            fd_ = -1;
        }
    }

    // frequently accessed fields
    uint32_t total_elements_;
    uint32_t element_size_log2_;
    uint32_t next_sequence_;
    uint32_t cached_reader_sequence_;
    char* buffer_;
    char* current_write_;
    bool shutdown_;

    // the following fields are rarely accessed
    uint32_t element_size_;
    uint32_t num_readers_;
    size_t buffer_size_;
    int fd_;

    std::string filename_;
};

} } }

#endif /* RING_WRITER_H */
