/*
 * qstream_common.hpp
 * Purpose: Common routines among different qstreams
 * Author: 
 */

#pragma once

#include "libcore/essential/platform_defs.hpp"
#include <string>

namespace miye
{
namespace qstream
{

enum class streamoption : uint8_t
{
    none = 0x0,
    read = 0x1,
    write = 0x2,
    wait = 0x4,
    follow = 0x8,
    timed = 0x10,
    adapter_ts = 0x20,
    must_create = 0x40,
    eof = 0x80
};
template <typename OS>
OS& operator<<(OS& os, streamoption s)
{
    switch (s)
    {
    case streamoption::none:
        os << "none";
        break;
    case streamoption::read:
        os << "read";
        break;
    case streamoption::write:
        os << "write";
        break;
    case streamoption::wait:
        os << "wait";
        break;
    case streamoption::follow:
        os << "follow";
        break;
    case streamoption::timed:
        os << "timed";
        break;
    case streamoption::adapter_ts:
        os << "adapter_ts";
        break;
    case streamoption::must_create:
        os << "must_create";
        break;
    case streamoption::eof:
        os << "eof";
        break;
    default:
        os << "unknown";
        break;
    }
    return os;
}

// separate to hold more than one streamoption
typedef uint8_t streamoptions;

enum class qstream_type_t : uint8_t
{
    /* readers */
    mmap_reader = 0,
    tcp_listener = 1,
    tcp_reader = 2,
    udp_reader = 3,
    nulltimer = 4,
    timer = 5,
    cycletimer = 6,
    gzfile = 7,
    exchangesim_reader = 8,
    pcap_reader = 9,
    ctp_csv_reader = 10,
    /* writers */
    mmap_writer = 0x80,
    tcp_writer = 0x82,
    udp_writer = 0x83,
    stdout_writer = 0x84,
    english_writer = 0x85,
    null_writer = 0x86,
    stats_writer = 0x87,
    exchangesim_writer = 0x88,
    pcap_writer = 0x89,

    undefined = 0xff
};
template <typename OS>
OS& operator<<(OS& os, qstream_type_t q)
{
    switch (q)
    {
    case qstream_type_t::mmap_reader:
        os << "mmfile_r";
        break;
    case qstream_type_t::tcp_listener:
        os << "tcp_l";
        break;
    case qstream_type_t::tcp_reader:
        os << "tcp_r";
        break;
    case qstream_type_t::udp_reader:
        os << "udp_r";
        break;
    case qstream_type_t::nulltimer:
        os << "nulltimer";
        break;
    case qstream_type_t::timer:
        os << "timer";
        break;
    case qstream_type_t::cycletimer:
        os << "cycletimer";
        break;
    case qstream_type_t::gzfile:
        os << "gzfile";
        break;
    case qstream_type_t::exchangesim_reader:
        os << "exchangesim_r";
        break;
    case qstream_type_t::pcap_reader:
        os << "pcap_r";
        break;
        /* writers */
    case qstream_type_t::mmap_writer:
        os << "mmfile_w";
        break;
    case qstream_type_t::tcp_writer:
        os << "tcp_w";
        break;
    case qstream_type_t::udp_writer:
        os << "udp_w";
        break;
    case qstream_type_t::stdout_writer:
        os << "stdout";
        break;
    case qstream_type_t::english_writer:
        os << "english";
        break;
    case qstream_type_t::null_writer:
        os << "null";
        break;
    case qstream_type_t::stats_writer:
        os << "stats";
        break;
    case qstream_type_t::exchangesim_writer:
        os << "exchangesim_w";
        break;
    case qstream_type_t::pcap_writer:
        os << "pcap_w";
        break;
    case qstream_type_t::ctp_csv_reader:
        os << "ctp_csv_reader_r";
        break;
    case qstream_type_t::undefined:
        os << "undefined";
        break;
    }
    return os;
}
qstream_type_t from_str(std::string stream_type_desc);
std::string extract_stream_type(std::string description);
std::string extract_path(std::string description);
streamoptions extract_streamoptions(std::string description);
std::string extract_val_for_key(std::string description, std::string key);
uint32_t extract_val_modifier(std::string val_string);
uint32_t extract_recordsize(std::string description);
uint32_t extract_promisc(std::string description);
uint64_t extract_latency(std::string description);
size_t extract_initial_filesize(std::string description, size_t default_size);
size_t extract_mappingsize(std::string description, size_t default_size);
std::string extract_filter(std::string description);
std::string extract_sim_exchange_type(std::string description);
bool is_anticipate(std::string description);

bool inline is_set(streamoption to_test, streamoptions options) noexcept
{
    return options & static_cast<streamoptions>(to_test);
}
bool inline follows(streamoptions options) noexcept
{
    return is_set(streamoption::follow, options);
}
HOT inline bool is_timed(streamoptions options) noexcept
{
    return is_set(streamoption::timed, options);
}
HOT inline bool is_adapter_ts(streamoptions options) noexcept
{
    return is_set(streamoption::adapter_ts, options);
}
HOT inline bool is_wait(streamoptions options) noexcept
{
    return is_set(streamoption::wait, options);
}
HOT inline bool must_create_set(streamoptions options) noexcept
{
    return is_set(streamoption::must_create, options);
}

} // namespace qstream
} // namespace miye
