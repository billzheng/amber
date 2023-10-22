
/*
 * pcap_reader.hpp
 * purpose: libpcap sniffing on an interface
 * Author: 
 */
//#LINKFLAGS=-lpcap

#pragma once
#include "arbiter_common.hpp"
#include "libcore/essential/platform_defs.hpp"
#include "libcore/time/timeutils.hpp"
#include "libcore/utils/fileutils.hpp"
#include "qstream_common.hpp"
#include "qstream_reader_interface.hpp"

#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <pcap.h>

#define SIZE_512M (512 * 1024 * 1024)

namespace miye
{
namespace qstream
{
template <typename Clock>
class pcap_reader : public qstream_reader_interface<pcap_reader<Clock>>
{
  public:
    pcap_reader(Clock& clk_, std::string description_)
        : clk(clk_), description(description_), hit_eof(false)
    {
        std::string interface = extract_path(description);
        bool is_file = utils::file_exists(interface);
        std::string filter = extract_filter(description);
        auto options = extract_streamoptions(description);
        timed = is_timed(options);

        adapter_ts = is_adapter_ts(options);

        int promisc = extract_promisc(description);
        // snaplen should be as small as possible but at least as big
        // as the largest packet expected
        static const int snaplen = (2 * 1024);
        char errbuf[PCAP_ERRBUF_SIZE];
        errbuf[0] = 0;
        static const int to_ms = 0;
        if (is_file)
        {
            std::cerr << "using pcap_open_offline()\n";
            pcap_handle = ::pcap_open_offline_with_tstamp_precision(
                interface.c_str(), PCAP_TSTAMP_PRECISION_NANO, errbuf);
            INVARIANT_MSG(pcap_handle,
                          "failed to open_offline"
                              << interface << " and get pcap_handle");
        }
        else
        {
            std::cerr << "using pcap_create()\n";
            pcap_handle = ::pcap_create(interface.c_str(), errbuf);
            INVARIANT_MSG(pcap_handle,
                          __PRETTY_FUNCTION__ << " failed to open device "
                                              << interface.c_str() << "\n"
                                              << errbuf);

            auto ret = pcap_set_tstamp_precision(pcap_handle,
                                                 PCAP_TSTAMP_PRECISION_NANO);
            if (ret)
            {
                std::cerr << "failed to set nanosecond precision " << DUMP(ret)
                          << DUMP(PCAP_ERROR_TSTAMP_PRECISION_NOTSUP)
                          << std::endl;
            }
            ret = pcap_set_snaplen(pcap_handle, snaplen);
            if (ret)
            {
                std::cerr << "failed to set snaplen " << DUMP(ret)
                          << DUMP(snaplen) << std::endl;
            }
            ret = pcap_set_promisc(pcap_handle, promisc);
            if (ret)
            {
                std::cerr << "failed to set promisc " << DUMP(ret)
                          << DUMP(promisc) << std::endl;
            }
            if (adapter_ts)
            {
                ret = pcap_set_tstamp_type(pcap_handle,
                                           PCAP_TSTAMP_ADAPTER_UNSYNCED);
                if (ret)
                {
                    std::cerr
                        << "failed to set tstamp type to adapter_unsynced "
                        << DUMP(ret) << std::endl;
                }
            }
            else
            {
                ret = pcap_set_tstamp_type(pcap_handle, PCAP_TSTAMP_HOST);
                if (ret)
                {
                    std::cerr << "failed to set tstamp type to host "
                              << DUMP(ret) << std::endl;
                }
            }
            ret = pcap_set_timeout(pcap_handle, to_ms);
            if (ret)
            {
                std::cerr << "failed to set timeout to 0 " << DUMP(ret)
                          << std::endl;
            }
            ret = pcap_set_buffer_size(pcap_handle, SIZE_512M);
            if (ret)
            {
                std::cerr << "failed to set buffer to 256 Meg " << DUMP(ret)
                          << std::endl;
            }
            ret = pcap_activate(pcap_handle);
            INVARIANT_MSG(!ret, "failed to activate pcap_handle " << DUMP(ret));
        }
        auto prec = pcap_get_tstamp_precision(pcap_handle);
        if (prec == PCAP_TSTAMP_PRECISION_NANO)
        {
            std::cerr << "pcap has nano precision\n";
        }
        else if (prec == PCAP_TSTAMP_PRECISION_MICRO)
        {
            std::cerr << "pcap has micro precision\n";
        }
        else
        {
            std::cerr << "pcap has unknown precision\n";
        }
        fd = ::pcap_get_selectable_fd(pcap_handle);
        if (!filter.empty())
        {
            static const int optimise = 1;
            // compile the filter and set it - if either fails it's fatal
            INVARIANT_MSG(
                ::pcap_compile(pcap_handle,
                               &fp,
                               filter.c_str(),
                               optimise,
                               PCAP_NETMASK_UNKNOWN) >= 0,
                "failed to compile berkley packet filter: " << filter);

            INVARIANT_MSG(::pcap_setfilter(pcap_handle, &fp) >= 0,
                          "compiled but failed to set filter: " << filter);
        }
    }

    const place read()
    {
#if defined(KERNEL_LEVEL_PCAP_ARBITRATION)

        get_data();
#else

#endif
        if (hit_eof)
        {
            return place::eof();
        }
        // data was read into pcap_data buffer when attest called
        // and there was something there
        // after last ready
        if (timed)
        {
            clk.set(last_ts_seen);
        }
        return place((char*)pcap_data, pcap_hdr->caplen);
    }

    void slow_attest(uint64_t* next_timestamp)
    {
        attest(next_timestamp);
    }
    void attest(uint64_t* next_timestamp)
    {
#if !defined(KERNEL_LEVEL_PCAP_ARBITRATION)
        if (*next_timestamp == recheck_timestamp)
        {
            get_data();
            if (last_ts_seen)
            {
                *next_timestamp = last_ts_seen;
            }
        }
#else
        // does nothing if we rely on epoll
        // to know when it's ready
        UNUSED(next_timestamp);
#endif
    }

    void get_data()
    {
        int pcap_retval = ::pcap_next_ex(pcap_handle, &pcap_hdr, &pcap_data);
        if (pcap_retval == -1)
        {
            // error
            INVARIANT_FAIL("pcap read error: " << ::pcap_geterr(pcap_handle));
        }
        else if (pcap_retval == -2)
        {
            // eof from pcap file read
            hit_eof = true;
            last_ts_seen = clk.now();
        }
        else if (pcap_retval == 0)
        {
            // timeout expired when reading
            // from live device
            // -- do nothing
            last_ts_seen = 0;
        }
        else
        {
            INVARIANT(pcap_retval > 0);
            // something was read, set the timestamp
            // note as we set nanosecond precision, the tv_usec field is filled
            // with nanos, not micros
            last_ts_seen = time::seconds(pcap_hdr->ts.tv_sec) +
                           time::nanos(pcap_hdr->ts.tv_usec);
        }
    }

    int get_fd()
    {
        return fd;
    }

    Clock& clk;
    const std::string description;
    bool timed{false};
    bool adapter_ts{false};
    pcap_t* pcap_handle;
    int fd{0};
    bpf_program fp{};
    pcap_pkthdr* pcap_hdr{nullptr};
    uint64_t last_ts_seen{0};
    const u_char* pcap_data{nullptr};
    bool hit_eof{false};
};
} // namespace qstream
} // namespace miye
