/*
 * pcap_headers.hpp
 * struct definitions for libcap capture fuile format
 * https://wiki.wireshark.org/Development/LibpcapFileFormat
 */

#pragma once
#include "stdint.h"

namespace miye
{
namespace qstream
{

typedef struct pcap_hdr_s
{
    uint32_t magic_number;  /* magic number */
    uint16_t version_major; /* major version number */
    uint16_t version_minor; /* minor version number */
    int32_t thiszone;       /* GMT to local correction */
    uint32_t sigfigs;       /* accuracy of timestamps */
    uint32_t snaplen;       /* max length of captured packets, in octets */
    uint32_t network;       /* data link type */
} pcap_hdr_t;

static const uint32_t NANO_PREC_MAGIC = 0xa1b23c4d;
static const uint16_t VERSION_MAJOR = 2;
static const uint16_t VERSION_MINOR = 4;
static const uint32_t THIS_ZONE_UTC = 0;
static const uint32_t SIGFIGS = 0;
static const uint32_t LINK_ETHERNET = 1;

template <typename OS>
OS& operator<<(OS& os, pcap_hdr_t p)
{
    os << "magic_number: " << p.magic_number;
    if (p.magic_number == NANO_PREC_MAGIC)
    {
        os << " nano precision";
    }
    os << " version_major: " << p.version_major;
    os << " version_minor: " << p.version_minor;
    os << " sigfigs: " << p.sigfigs;
    os << " snaplen: " << p.snaplen;
    os << " network: " << p.network;
    if (p.network == LINK_ETHERNET)
    {
        os << " ethernet";
    }
    return os;
}

inline pcap_hdr_t prime_pcap_hdr(uint32_t snaplen)
{
    pcap_hdr_t p;
    p.magic_number = NANO_PREC_MAGIC;
    p.version_major = VERSION_MAJOR;
    p.version_minor = VERSION_MINOR;
    p.thiszone = THIS_ZONE_UTC;
    p.sigfigs = SIGFIGS;
    p.snaplen = snaplen;
    p.network = LINK_ETHERNET;
    return p;
}

typedef struct pcaprec_hdr_s
{
    uint32_t ts_sec;   /* timestamp seconds */
    uint32_t ts_usec;  /* timestamp microseconds */
    uint32_t incl_len; /* number of octets of packet saved in file */
    uint32_t orig_len; /* actual length of packet */
} pcaprec_hdr_t;

} // namespace qstream
} // namespace miye
