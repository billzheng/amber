/*
 * variantqstream_writer.hpp
 * purpose: write any qstream
 * Author: 
 */

#pragma once
#include "english_writer.hpp"
#include "exchangesim_writer.hpp"
#include "libcore/essential/assert.hpp"
#include "mmap_writer.hpp"
#include "null_writer.hpp"
#include "pcap_writer.hpp"
#include "qstream_common.hpp"
#include "qstream_writer_interface.hpp"
#include "stats_writer.hpp"
#include "stdout_writer.hpp"
#include "tcp_writer.hpp"
#include "udp_writer.hpp"
#include <memory>

namespace miye
{
namespace qstream
{

template <typename Clock>
static void variant_mmap_w_deleter(void* ptr)
{
    static_cast<mmap_writer<Clock>*>(ptr)->~mmap_writer();
    free(ptr);
}

template <typename Clock>
struct variantqstream_writer
    : public qstream_writer_interface<variantqstream_writer<Clock>>
{
    typedef mmap_writer<Clock> mmap_writer_t;
    typedef tcp_writer<Clock> tcp_writer_t;
    typedef udp_writer<Clock> udp_writer_t;
    typedef stdout_writer<Clock> stdout_writer_t;
    typedef english_writer<Clock> english_writer_t;
    typedef null_writer<Clock> null_writer_t;
    typedef stats_writer<Clock> stats_writer_t;
    typedef exchangesim_writer<Clock> exchangesim_writer_t;
    typedef pcap_writer<Clock> pcap_writer_t;
    variantqstream_writer()
        : qstream_obj(nullptr), qstream_type(qstream_type_t::undefined),
          created(false)
    {
    }
    variantqstream_writer(Clock& clock, std::string descrip) : created(true)
    {
        std::string stream_t_desc = extract_stream_type(descrip);
        qstream_type = from_str(stream_t_desc.c_str());
        switch (qstream_type)
        {
        case qstream_type_t::mmap_writer:
            void* allocated;
            syscalls::posix_memalign(
                &allocated, CACHE_LINE_SIZE, sizeof(mmap_writer_t));
            INVARIANT_ALIGNED(allocated, CACHE_LINE_SIZE);
            qstream_obj.reset(new (allocated) mmap_writer_t(clock, descrip),
                              variant_mmap_w_deleter<Clock>);
            break;
        case qstream_type_t::tcp_writer:
            qstream_obj.reset(new tcp_writer_t(clock, descrip));
            break;
        case qstream_type_t::udp_writer:
            qstream_obj.reset(new udp_writer_t(clock, descrip));
            break;
        case qstream_type_t::stdout_writer:
            qstream_obj.reset(new stdout_writer_t(clock, descrip));
            break;
        case qstream_type_t::english_writer:
            qstream_obj.reset(new english_writer_t(clock, descrip));
            break;
        case qstream_type_t::null_writer:
            qstream_obj.reset(new null_writer_t(clock, descrip));
            break;
        case qstream_type_t::stats_writer:
            qstream_obj.reset(new stats_writer_t(clock, descrip));
            break;
        case qstream_type_t::exchangesim_writer:
            qstream_obj.reset(new exchangesim_writer_t(clock, descrip));
            break;
        case qstream_type_t::pcap_writer:
            qstream_obj.reset(new pcap_writer_t(clock, descrip));
            break;
        default:
            INVARIANT_FAIL(
                DUMP(stream_t_desc)
                << DUMP(qstream_type) << " from " << DUMP(descrip)
                << " is not handled by variantqstream_writer"
                << (int)(qstream_type == qstream_type_t::pcap_writer));
            break;
        }
    }

    const std::string& describe() const
    {
        switch (qstream_type)
        {
        case qstream_type_t::mmap_writer:
            return reinterpret_cast<mmap_writer_t*>(qstream_obj.get())
                ->describe();
            break;
        case qstream_type_t::tcp_writer:
            return reinterpret_cast<tcp_writer_t*>(qstream_obj.get())
                ->describe();
            break;
        case qstream_type_t::udp_writer:
            return reinterpret_cast<udp_writer_t*>(qstream_obj.get())
                ->describe();
            break;
        case qstream_type_t::stdout_writer:
            return reinterpret_cast<stdout_writer_t*>(qstream_obj.get())
                ->describe();
            break;
        case qstream_type_t::english_writer:
            return reinterpret_cast<english_writer_t*>(qstream_obj.get())
                ->describe();
            break;
        case qstream_type_t::null_writer:
            return reinterpret_cast<null_writer_t*>(qstream_obj.get())
                ->describe();
            break;
        case qstream_type_t::stats_writer:
            return reinterpret_cast<stats_writer_t*>(qstream_obj.get())
                ->describe();
            break;
        case qstream_type_t::exchangesim_writer:
            return reinterpret_cast<exchangesim_writer_t*>(qstream_obj.get())
                ->describe();
            break;
        case qstream_type_t::pcap_writer:
            return reinterpret_cast<pcap_writer_t*>(qstream_obj.get())
                ->describe();
            break;
        default:
            INVARIANT_FAIL(qstream_type
                           << " is not handled by variantqstream_writer");
            break;
        }
        return description;
    }

    place pledge(size_t n)
    {
        switch (qstream_type)
        {
        case qstream_type_t::mmap_writer:
            return reinterpret_cast<mmap_writer_t*>(qstream_obj.get())
                ->pledge(n);
            break;
        case qstream_type_t::tcp_writer:
            return reinterpret_cast<tcp_writer_t*>(qstream_obj.get())
                ->pledge(n);
            break;
        case qstream_type_t::udp_writer:
            return reinterpret_cast<udp_writer_t*>(qstream_obj.get())
                ->pledge(n);
            break;
        case qstream_type_t::stdout_writer:
            return reinterpret_cast<stdout_writer_t*>(qstream_obj.get())
                ->pledge(n);
            break;
        case qstream_type_t::english_writer:
            return reinterpret_cast<english_writer_t*>(qstream_obj.get())
                ->pledge(n);
            break;
        case qstream_type_t::null_writer:
            return reinterpret_cast<null_writer_t*>(qstream_obj.get())
                ->pledge(n);
            break;
        case qstream_type_t::stats_writer:
            return reinterpret_cast<stats_writer_t*>(qstream_obj.get())
                ->pledge(n);
            break;
        case qstream_type_t::exchangesim_writer:
            return reinterpret_cast<exchangesim_writer_t*>(qstream_obj.get())
                ->pledge(n);
            break;
        case qstream_type_t::pcap_writer:
            return reinterpret_cast<pcap_writer_t*>(qstream_obj.get())
                ->pledge(n);
            break;
        default:
            INVARIANT_FAIL(qstream_type
                           << " unhandled by variantqstream_writer");
            break;
        }
        // silence compiler warning
        return qstream::place::eof();
    }

    int announce(const place& p)
    {
        int retval = -1;
        switch (qstream_type)
        {
        case qstream_type_t::mmap_writer:
            retval = reinterpret_cast<mmap_writer_t*>(qstream_obj.get())
                         ->announce(p);
            break;
        case qstream_type_t::tcp_writer:
            retval =
                reinterpret_cast<tcp_writer_t*>(qstream_obj.get())->announce(p);
            break;
        case qstream_type_t::udp_writer:
            retval =
                reinterpret_cast<udp_writer_t*>(qstream_obj.get())->announce(p);
            break;
        case qstream_type_t::stdout_writer:
            retval = reinterpret_cast<stdout_writer_t*>(qstream_obj.get())
                         ->announce(p);
            break;
        case qstream_type_t::english_writer:
            retval = reinterpret_cast<english_writer_t*>(qstream_obj.get())
                         ->announce(p);
            break;
        case qstream_type_t::null_writer:
            retval = reinterpret_cast<null_writer_t*>(qstream_obj.get())
                         ->announce(p);
            break;
        case qstream_type_t::stats_writer:
            retval = reinterpret_cast<stats_writer_t*>(qstream_obj.get())
                         ->announce(p);
            break;
        case qstream_type_t::exchangesim_writer:
            retval = reinterpret_cast<exchangesim_writer_t*>(qstream_obj.get())
                         ->announce(p);
            break;
        case qstream_type_t::pcap_writer:
            retval = reinterpret_cast<pcap_writer_t*>(qstream_obj.get())
                         ->announce(p);
            break;
        default:
            INVARIANT_FAIL(qstream_type
                           << " unhandled by variantqstream_writer");
            break;
        }
        return retval;
    }
    int write(const void* p, size_t n)
    {
        switch (qstream_type)
        {
        case qstream_type_t::mmap_writer:
            return reinterpret_cast<mmap_writer_t*>(qstream_obj.get())
                ->write(p, n);
        case qstream_type_t::tcp_writer:
            return reinterpret_cast<tcp_writer_t*>(qstream_obj.get())
                ->write(p, n);
        case qstream_type_t::udp_writer:
            return reinterpret_cast<udp_writer_t*>(qstream_obj.get())
                ->write(p, n);
        case qstream_type_t::stdout_writer:
            return reinterpret_cast<stdout_writer_t*>(qstream_obj.get())
                ->write(p, n);
        case qstream_type_t::english_writer:
            return reinterpret_cast<english_writer_t*>(qstream_obj.get())
                ->write(p, n);
        case qstream_type_t::null_writer:
            return reinterpret_cast<null_writer_t*>(qstream_obj.get())
                ->write(p, n);
        case qstream_type_t::stats_writer:
            return reinterpret_cast<stats_writer_t*>(qstream_obj.get())
                ->write(p, n);
        case qstream_type_t::exchangesim_writer:
            return reinterpret_cast<exchangesim_writer_t*>(qstream_obj.get())
                ->write(p, n);
        case qstream_type_t::pcap_writer:
            return reinterpret_cast<pcap_writer_t*>(qstream_obj.get())
                ->write(p, n);
        default:
            INVARIANT_FAIL(qstream_type
                           << " unhandled by variantqstream_writer");
            break;
        }
        return false;
    }

    int get_fd()
    {
        switch (qstream_type)
        {
        case qstream_type_t::tcp_writer:
            return reinterpret_cast<tcp_writer_t*>(qstream_obj.get())->get_fd();
            break;
        case qstream_type_t::udp_reader:
            return reinterpret_cast<udp_writer_t*>(qstream_obj.get())->get_fd();
            break;
        default:
            break;
        }
        INVARIANT_FAIL("get_fd() called on non-supporting object");
        return -1;
    }

    template <typename T>
    T* as()
    {
        return reinterpret_cast<T*>(qstream_obj.get());
    }

    qstream_type_t get_type()
    {
        return qstream_type;
    }

    ~variantqstream_writer()
    {
        // std::cerr << "destructor called on variant writer with underlying: "
        // << qstream_type << std::endl;
    }

  private:
    std::shared_ptr<void> qstream_obj;
    qstream_type_t qstream_type;
    bool created;
    static const std::string description;
};

template <typename Clock>
const std::string variantqstream_writer<Clock>::description = "no description";

} // namespace qstream
} // namespace miye
