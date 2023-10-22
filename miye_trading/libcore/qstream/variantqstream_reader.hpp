/*
 * variantqstream_reader.hpp
 * purpose: read any qstream
 * Author:
 */

#pragma once
#include "cycletimer.hpp"
#include "exchangesim_reader.hpp"
#include "exchangesim_writer.hpp"
#include "gzfile.hpp"
#include "libcore/essential/assert.hpp"
#include "mmap_reader.hpp"
#include "nulltimer.hpp"
#include "pcap_reader.hpp"
#include "qstream_common.hpp"
#include "qstream_reader_interface.hpp"
#include "tcp_listener.hpp"
#include "tcp_reader.hpp"
#include "timer.hpp"
#include "udp_reader.hpp"
#include "variantqstream_writer.hpp"
#include <memory>

namespace miye
{
namespace qstream
{

template <typename Clock>
static void variant_mmap_r_deleter(void* ptr)
{
    static_cast<mmap_reader<Clock>*>(ptr)->~mmap_reader();
    free(ptr);
}

template <typename Clock>
struct variantqstream_reader : public qstream_reader_interface<variantqstream_reader<Clock>>
{
    typedef mmap_reader<Clock> mmap_reader_t;
    typedef tcp_listener<Clock> tcp_listener_t;
    typedef tcp_reader<Clock> tcp_reader_t;
    typedef udp_reader<Clock> udp_reader_t;
    typedef timer<Clock> timer_t;
    typedef cycletimer<Clock> cycletimer_t;
    typedef nulltimer<Clock> nulltimer_t;
    typedef gzfile<Clock> gzfile_t;
    typedef exchangesim_reader<Clock> exchangesim_reader_t;
    typedef exchangesim_writer<Clock> exchangesim_writer_t;
    typedef variantqstream_writer<Clock> variantqstream_writer_t;
    typedef pcap_reader<Clock> pcap_t;
    typedef ctp_csv_reader<Clock> ctp_csv_reader_t;

    variantqstream_reader() : qstream_obj(nullptr), qstream_type(qstream_type_t::undefined), created(false) {}

    variantqstream_reader(Clock& clock, std::string descrip) : created(true)
    {
        std::string stream_type_desc = extract_stream_type(descrip);
        qstream_type                 = from_str(stream_type_desc);
        switch (qstream_type)
        {
        case qstream_type_t::mmap_reader:
            void* allocated;
            syscalls::posix_memalign(&allocated, CACHE_LINE_SIZE, sizeof(mmap_reader_t));
            qstream_obj.reset(new (allocated) mmap_reader_t(clock, descrip), variant_mmap_r_deleter<Clock>);
            break;
        case qstream_type_t::tcp_listener:
            // only a tcp listener can be generated from description string
            // tcp reader is created from a writer or a listener
            // then passed py pointer to a constructor above
            qstream_obj.reset(new tcp_listener_t(clock, descrip));
            break;
        case qstream_type_t::udp_reader:
            qstream_obj.reset(new udp_reader_t(clock, descrip));
            break;
        case qstream_type_t::nulltimer:
            qstream_obj.reset(new nulltimer_t(clock, descrip));
            break;
        case qstream_type_t::cycletimer:
            qstream_obj.reset(new cycletimer_t(clock, descrip));
            break;
        case qstream_type_t::timer:
            qstream_obj.reset(new timer_t(clock, descrip));
            break;
        case qstream_type_t::gzfile:
            qstream_obj.reset(new gzfile_t(clock, descrip));
            break;
        case qstream_type_t::pcap_reader:
            qstream_obj.reset(new pcap_t(clock, descrip));
            break;
        case qstream_type_t::ctp_csv_reader:
            qstream_obj.reset(new ctp_csv_reader_t(clock, descrip));
            break;
        default:
            // note exchangesim_reader not constructable by description
            // so not defined above
            INVARIANT_FAIL(DUMP(qstream_type) << " from " << DUMP(descrip) << " is not handled by variantqstream");
            break;
        }
    }

    // we assume if we have an fd in the constructor it's tcp
    // xxx extend to udp
    variantqstream_reader(Clock& clock, int fd, std::string descrip) : created(true)
    {
        std::string stream_type_desc = extract_stream_type(descrip);
        qstream_type                 = from_str(stream_type_desc);
        INVARIANT_MSG(qstream_type == qstream_type_t::tcp_reader,
                      "Only valid to provide fd with tcp_stream construction");
        qstream_obj.reset(new tcp_reader_t(clock, fd, descrip));
    }

    // exchangesim reader is just a reading interface
    // onto the data held by the exchangesim writer
    // the writer has orders and md written to it
    // but the responses to those orders are read
    // from the exchangesim reader object
    variantqstream_reader(Clock& clock, exchangesim_writer_t& ex_writer)
    {
        qstream_obj.reset(new exchangesim_reader_t(ex_writer));
        qstream_type = qstream_type_t::exchangesim_reader;
        created      = true;
    }

    void fast_forward(uint64_t stop_ts = 0)
    {
        if (qstream_type == qstream_type_t::mmap_reader)
        {
            reinterpret_cast<mmap_reader_t*>(qstream_obj.get())->fast_forward(stop_ts);
        }
    }

    const std::string& describe() const
    {
        switch (qstream_type)
        {
        case qstream_type_t::mmap_reader:
            return reinterpret_cast<mmap_reader_t*>(qstream_obj.get())->describe();
            break;
        case qstream_type_t::tcp_listener:
            return reinterpret_cast<tcp_listener_t*>(qstream_obj.get())->describe();
            break;
        case qstream_type_t::tcp_reader:
            return reinterpret_cast<tcp_reader_t*>(qstream_obj.get())->describe();
            break;
        case qstream_type_t::udp_reader:
            return reinterpret_cast<udp_reader_t*>(qstream_obj.get())->describe();
            break;
        case qstream_type_t::nulltimer:
            return reinterpret_cast<nulltimer_t*>(qstream_obj.get())->describe();
            break;
        case qstream_type_t::timer:
            return reinterpret_cast<timer_t*>(qstream_obj.get())->describe();
            break;
        case qstream_type_t::cycletimer:
            return reinterpret_cast<cycletimer_t*>(qstream_obj.get())->describe();
            break;
        case qstream_type_t::gzfile:
            return reinterpret_cast<gzfile_t*>(qstream_obj.get())->describe();
            break;
        case qstream_type_t::exchangesim_reader:
            return reinterpret_cast<exchangesim_reader_t*>(qstream_obj.get())->describe();
            break;
        case qstream_type_t::pcap_reader:
            return reinterpret_cast<pcap_t*>(qstream_obj.get())->describe();
            break;
        case qstream_type_t::ctp_csv_reader:
            return reinterpret_cast<ctp_csv_reader_t*>(qstream_obj.get())->describe();
            break;
        default:
            INVARIANT_FAIL("unhandled stream type: " << DUMP(qstream_type));
            break;
        }
        return description;
    }

    const place read()
    {
        switch (qstream_type)
        {
        case qstream_type_t::mmap_reader:
            return reinterpret_cast<mmap_reader_t*>(qstream_obj.get())->read();
            break;
        case qstream_type_t::tcp_listener:
            return reinterpret_cast<tcp_listener_t*>(qstream_obj.get())->read();
            break;
        case qstream_type_t::tcp_reader:
            return reinterpret_cast<tcp_reader_t*>(qstream_obj.get())->read();
            break;
        case qstream_type_t::udp_reader:
            return reinterpret_cast<udp_reader_t*>(qstream_obj.get())->read();
            break;
        case qstream_type_t::nulltimer:
            return reinterpret_cast<nulltimer_t*>(qstream_obj.get())->read();
            break;
        case qstream_type_t::timer:
            return reinterpret_cast<timer_t*>(qstream_obj.get())->read();
            break;
        case qstream_type_t::cycletimer:
            return reinterpret_cast<cycletimer_t*>(qstream_obj.get())->read();
            break;
        case qstream_type_t::gzfile:
            return reinterpret_cast<gzfile_t*>(qstream_obj.get())->read();
            break;
        case qstream_type_t::exchangesim_reader:
            return reinterpret_cast<exchangesim_reader_t*>(qstream_obj.get())->read();
            break;
        case qstream_type_t::pcap_reader:
            return reinterpret_cast<pcap_t*>(qstream_obj.get())->read();
            break;
        case qstream_type_t::ctp_csv_reader:
            return reinterpret_cast<ctp_csv_reader_t*>(qstream_obj.get())->read();
            break;
        default:
            INVARIANT_FAIL("unhandled stream type: " << DUMP(qstream_type));
            break;
        }
        return place::eof();
    }

    void slow_attest(uint64_t* next_timestamp)
    {
        if (qstream_type == qstream_type_t::mmap_reader)
        {
            reinterpret_cast<mmap_reader_t*>(qstream_obj.get())->slow_attest(next_timestamp);
        }
        else
        {
            attest(next_timestamp);
        }
    }
    void attest(uint64_t* next_timestamp)
    {
        switch (qstream_type)
        {
        case qstream_type_t::mmap_reader:
            reinterpret_cast<mmap_reader_t*>(qstream_obj.get())->attest(next_timestamp);
            break;
        case qstream_type_t::tcp_listener:
            reinterpret_cast<tcp_listener_t*>(qstream_obj.get())->attest(next_timestamp);
            break;
        case qstream_type_t::tcp_reader:
            reinterpret_cast<tcp_reader_t*>(qstream_obj.get())->attest(next_timestamp);
            break;
        case qstream_type_t::udp_reader:
            reinterpret_cast<udp_reader_t*>(qstream_obj.get())->attest(next_timestamp);
            break;
        case qstream_type_t::nulltimer:
            reinterpret_cast<nulltimer_t*>(qstream_obj.get())->attest(next_timestamp);
            break;
        case qstream_type_t::timer:
            reinterpret_cast<timer_t*>(qstream_obj.get())->attest(next_timestamp);
            break;
        case qstream_type_t::cycletimer:
            reinterpret_cast<cycletimer_t*>(qstream_obj.get())->attest(next_timestamp);
            break;
        case qstream_type_t::gzfile:
            reinterpret_cast<gzfile_t*>(qstream_obj.get())->attest(next_timestamp);
            break;
        case qstream_type_t::exchangesim_reader:
            reinterpret_cast<exchangesim_reader_t*>(qstream_obj.get())->attest(next_timestamp);
            break;
        case qstream_type_t::pcap_reader:
            reinterpret_cast<pcap_t*>(qstream_obj.get())->attest(next_timestamp);
            break;
        case qstream_type_t::ctp_csv_reader:
            return reinterpret_cast<ctp_csv_reader_t*>(qstream_obj.get())->attest(next_timestamp);
            break;
        default:
            INVARIANT_FAIL("unhandled stream type: " << DUMP(qstream_type));
            break;
        }
    }

    bool is_kernel()
    {
        return qstream_type != qstream_type_t::mmap_reader && qstream_type != qstream_type_t::nulltimer &&
               qstream_type != qstream_type_t::timer && qstream_type != qstream_type_t::cycletimer &&
               qstream_type != qstream_type_t::gzfile
#if !defined(KERNEL_LEVEL_PCAP_ARBITRATION)
               && qstream_type != qstream_type_t::pcap_reader
#endif
               && qstream_type != qstream_type_t::exchangesim_reader && qstream_type != qstream_type_t::ctp_csv_reader;
    }
    bool is_mmap() { return qstream_type == qstream_type_t::mmap_reader; }
    bool is_gzfile() { return qstream_type == qstream_type_t::gzfile; }
    bool is_tcp_listener() { return qstream_type == qstream_type_t::tcp_listener; }
    qstream_type_t get_type() { return qstream_type; }
    bool is_valid() { return get_type() != qstream_type_t::undefined; }

    int get_fd()
    {
        if (is_kernel())
        {
            switch (qstream_type)
            {
            case qstream_type_t::tcp_listener:
                return reinterpret_cast<tcp_listener_t*>(qstream_obj.get())->get_fd();
                break;
            case qstream_type_t::tcp_reader:
                return reinterpret_cast<tcp_reader_t*>(qstream_obj.get())->get_fd();
                break;
            case qstream_type_t::udp_reader:
                return reinterpret_cast<udp_reader_t*>(qstream_obj.get())->get_fd();
                break;
#if defined(KERNEL_LEVEL_PCAP_ARBITRATION)
            case qstream_type_t::pcap_reader:
                return reinterpret_cast<pcap_t*>(qstream_obj.get())->get_fd();
                break;
#endif
            default:
                break;
            }
        }
        INVARIANT_FAIL("get_fd() called on non-kernel object " << qstream_type << this->describe());
        return -1;
    }

    uint64_t last_write_ts()
    {
        switch (qstream_type)
        {
        case qstream_type_t::mmap_reader:
            return reinterpret_cast<mmap_reader_t*>(qstream_obj.get())->last_write_ts();
            break;
        case qstream_type_t::gzfile:
            return reinterpret_cast<gzfile_t*>(qstream_obj.get())->last_write_ts();
            break;
        default:
            return 0;
            break;
        }
    }

    template <typename T>
    T* as()
    {
        return reinterpret_cast<T*>(qstream_obj.get());
    }

    ~variantqstream_reader()
    {
        // std::cerr << "variant destructor called on underlying: " <<
        // qstream_type << std::endl;
    }

  private:
    std::shared_ptr<void> qstream_obj;
    qstream_type_t qstream_type;
    bool created;
    static const std::string description;
};

template <typename Clock>
const std::string variantqstream_reader<Clock>::description = "no description";

} // namespace qstream
} // namespace miye
