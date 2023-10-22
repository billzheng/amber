#ifndef LIB_RING_READER_H
#define LIB_RING_READER_H

#include <string>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/file.h>
#include <exception>
#include <csignal>

#include "ring_common.h"

namespace dinobot { namespace lib { namespace shm {

struct ring_reader 
{
    ring_reader(std::string filename, uint32_t reader_id)
        : header_(NULL),
          reader_record_(NULL),
          elements_(NULL),
	      next_packet_(NULL),
	      next_packet_size_(0),
	      next_packet_writer_timestamp_(0),
	      next_packet_chunks_(0),
          has_next_(false),
          shutdown_(false),
          fd_(-1),
          reader_id_(reader_id),
          buffer_(NULL)//,
          //last_header_read_time_(base::timing::min().counter())
    {
        filename_ = filename;
        open_ring();

        //memset(&select_info_, 0, sizeof(select_info_));
        //select_info_.type = interfaces::select_info::SPIN;
        //select_info_.warn_if_slow = true;
    }

    ~ring_reader()
    {
        shutdown();
        close_ring();
    }

    bool ready()
    {
        return try_fetch_next() || shutdown_;
    }

    struct ring_reader_retval peek()
    {
		struct ring_reader_retval ret = {NULL, 0, 0, 0};

        if (unlikely(shutdown_))
            return ret;

        /* Optimise for the case where we called ready() first, then read() */
        if (likely(has_next_))
		{
			ret.buffer = next_packet_;
			ret.size = next_packet_size_;
            ret.writer_timestamp = next_packet_writer_timestamp_;
            ret.chunks = next_packet_chunks_;
			return ret;
		}

        while (!shutdown_)
            if (try_fetch_next())
			{
				if (has_next_)
				{
					ret.buffer = next_packet_;
					ret.size = next_packet_size_;
                    ret.writer_timestamp = next_packet_writer_timestamp_;
                    ret.chunks = next_packet_chunks_;
					return ret;
				}
				else 
					return ret;
			}

        return ret;
    }

    struct ring_reader_retval read()
    {
        struct ring_reader_retval r = peek();
        has_next_ = false;
        return r;
    }

    void shutdown()
    {
        if (reader_record_ != NULL)
            reader_record_->sequence = next_sequence_;
        shutdown_ = true;
    }

	/*
    const interfaces::select_info* select_info() const
    {
        return &select_info_;
    }
	*/

    std::string status() const
    {
        return "";
    }

private:
    /**
     * Tries to fetch the next packet, returns false if nothing ready.
     */
    bool try_fetch_next()
    {
        if (unlikely(has_next_ || shutdown_))
            return true;

        if (unlikely(elements_ == NULL))
            if (!open_ring())
                return false;

        // invalidate previous read
        reader_record_->sequence = next_sequence_;

        char* current_read = elements_ +
            ((next_sequence_ & (total_elements_ - 1)) << element_size_log2_);

        if (((ring_element_header*)current_read)->sequence == next_sequence_)
        {
            // data is ready - deliver the data to the user
            asm volatile("lfence");
            //next_packet_ = base::make_const_range(
            //        current_read + sizeof(ring_element_header),
            //        ((ring_element_header*)current_read)->size);
            next_packet_ = current_read + sizeof(ring_element_header);
			next_packet_size_ = ((ring_element_header*)current_read)->size;
			next_packet_writer_timestamp_ = ((ring_element_header*)current_read)->writer_timestamp;
			next_packet_chunks_ = ((ring_element_header*)current_read)->chunks;

            has_next_ = true;
            next_sequence_++;
            return true;
        }
        else
            return false;
    }

    /**
     * Tries to open a ring that has the requisite header information in it.
     *
     * Returns true if successful, and updates fd_ to contain the correct
     * file descriptor.
     */
    bool open_ring()
    {
        if (buffer_ != NULL)
            return true;

        // Make it so that you can't call open_ring more than once per second
		//base::timing::time_counter now = base::timing::utc.now();
        //if ((now - last_header_read_time_) < base::timing::seconds(1))
        //    return false;
        //last_header_read_time_ = now;

        if (fd_ != -1)
            ::close(fd_);

        fd_ = ::open(filename_.c_str(), O_RDWR);
        if (fd_ == -1)
            return false;

        struct stat s;
        ::fstat(fd_, &s);

        // if the file isn't as big as a ring page, it's not (yet) a valid ring file
        if ((size_t)s.st_size < ring_pagesize)
        {
            close_ring();
            return false;
        }

        // map in the header
        header_ = (ring_header*)::mmap(NULL, ring_header_pagesize,
                PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
        if (header_ == MAP_FAILED)
        {
            header_ = NULL;
            close_ring();
			throw std::runtime_error("header mmap failed");
        }

        // if magic is incorrect, retry later
        if (header_->ring_magic != ring_magic)
        {
            close_ring();
            return false;
        }

        // check version of ring format
        if (header_->version != ring_version_number ||
                (header_->element_size & (header_->element_size - 1)) != 0 ||
                (header_->total_elements & (header_->total_elements - 1)) != 0)
        {
			std::cout << "header_->version: " << header_->version 
					  << "header_->element_size: " << header_->element_size
					  << "header_->total_elements: " << header_->total_elements;

            ring_header header_copy = *header_;
            close_ring();

			std::stringstream ss;
            ss  << "invalid format:" 
				<< " version=" << header_copy.version 
				<< " element_size=" << header_copy.element_size 
				<< " total_elements=" << header_copy.total_elements;
			throw std::runtime_error(ss.str());
        }

        // check slot number is within range
        if (reader_id_ >= header_->num_readers)
        {
            ring_header header_copy = *header_;
            close_ring();

			std::stringstream ss;
            ss  << "invalid reader id: "  << reader_id_ 
				<< " (ring has " << header_copy.num_readers 
				<< " readers)";
			throw std::runtime_error(ss.str());
        }

        element_size_log2_ = int_log2(header_->element_size);
        total_elements_ = header_->total_elements;

        reader_record_ = (ring_reader_record*)((char*)header_ + ring_reader_offset
                + reader_id_ * ring_reader_record_size);

        // try to take the reader id
        uint32_t old_pid = 0;
        while (true)
        {
            uint32_t p = cmpxchg32(&reader_record_->pid, old_pid, getpid());
            if (p == old_pid)
                // success
                break;
            else if (kill(p, 0) != 0 && errno == ESRCH)
                // old reader is dead, try to take over the reader id
                old_pid = p;
            else
            {
                reader_record_ = NULL;
                close_ring();
			
				std::stringstream ss;
				ss  << "reader id: "  << reader_id_ 
					<< " already taken by pid " << p;
				throw std::runtime_error(ss.str());
            }
        }

        next_sequence_ = reader_record_->sequence;

        // map in the elements
        buffer_size_ = ring_buffer_size(header_->element_size, total_elements_);
        buffer_ = (char*)::mmap(NULL, buffer_size_, PROT_READ, MAP_SHARED, fd_, 0);
        if (buffer_ == MAP_FAILED)
        {
            close_ring();
			throw std::runtime_error("buffer mmap failed");
        }

        elements_ = buffer_ + ring_element_offset;

        // read from the start of each element to cause the mapping to be
        // loaded into the TLB
        for (size_t i = 0; i < total_elements_; i++)
        {
            size_t offset = i << element_size_log2_;
            volatile char c = *(volatile char*)(elements_ + offset);
            (void)c;
        }

        return true;
    }

    void close_ring()
    {
        if (reader_record_ != NULL)
            reader_record_->pid = 0;

        if (header_ != NULL)
            ::munmap(header_, ring_header_pagesize);

        if (buffer_ != NULL)
            ::munmap(buffer_, buffer_size_);

        if (fd_ != -1)
            ::close(fd_);

        buffer_ = NULL;
        header_ = NULL;
        elements_ = NULL;
        reader_record_ = NULL;
        fd_ = -1;
    }

    // frequently accessed fields
    ring_header* header_;
    ring_reader_record* reader_record_;
    char* elements_;
    uint32_t element_size_log2_;
    uint32_t total_elements_;
    uint32_t next_sequence_;
    //base::const_range next_packet_;
    const char *next_packet_;
	uint32_t next_packet_size_;
    uint64_t next_packet_writer_timestamp_;
    uint64_t next_packet_chunks_;
    bool has_next_;
    bool shutdown_;

    // less frequently accessed fields
    int fd_;
    uint32_t reader_id_;
    char* buffer_;
    size_t buffer_size_;
    std::string filename_;
    //interfaces::select_info select_info_;
};

} } }

#endif /* RING_READER_H */
