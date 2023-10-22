/*
 * qstream_common.cpp
 * Purpose: implement qstrem_common.hpp
 * Author: 
 */

#include "qstream_common.hpp"
#include "libcore/essential/assert.hpp"

namespace miye
{
namespace qstream
{

std::string extract_stream_type(std::string description)
{
    auto pos = description.find(":");
    return description.substr(0, pos);
}
std::string extract_path(std::string description)
{
    auto start_path = description.find(":");
    auto end_path = description.find("@");

    return description.substr(start_path + 1, end_path - start_path - 1);
}

streamoptions extract_streamoptions(std::string description)
{
    streamoptions opts = static_cast<streamoptions>(streamoption::none);
    auto end_path = description.find("@");
    auto options_str = description.substr(end_path + 1, std::string::npos);
    if (options_str.find("follow") != std::string::npos)
    {
        opts |= static_cast<streamoptions>(streamoption::follow);
    }
    if (options_str.find("timed") != std::string::npos)
    {
        opts |= static_cast<streamoptions>(streamoption::timed);
    }
    if (options_str.find("adapter_ts") != std::string::npos)
    {
        opts |= static_cast<streamoptions>(streamoption::adapter_ts);
    }
    if (options_str.find("wait") != std::string::npos)
    {
        opts |= static_cast<streamoptions>(streamoption::wait);
    }
    if (options_str.find("must_create") != std::string::npos)
    {
        opts |= static_cast<streamoptions>(streamoption::must_create);
    }

    return opts;
}
std::string extract_options_string(std::string description)
{
    auto end_path = description.find("@");
    return description.substr(end_path + 1, std::string::npos);
}

std::string extract_val_for_key(std::string description, std::string key)
{
    auto options_str = extract_options_string(description);
    auto pos = options_str.find(key);
    if (pos == std::string::npos)
    {
        // key not present
        return std::string("");
    }
    auto val_str = options_str.substr(pos + key.length(), std::string::npos);
    // terminates with a "," or it's the end of the description
    return val_str.substr(0, val_str.find(","));
}

uint32_t extract_val_modifier(std::string val_string)
{
    char modifier = val_string[val_string.length() - 1];
    uint32_t multiplier = 1;
    switch (modifier)
    {
    case 'k':
    case 'K':
        multiplier = 1024;
        break;
    case 'M':
    case 'm':
        multiplier = 1024 * 1024;
        break;
    case 'G':
    case 'g':
        multiplier = 1024 * 1024 * 1024;
        break;
    default:
        // otherwise last char should be something that is part of an ascii
        // number
        INVARIANT_MSG((modifier >= '0' && modifier <= '9') ||
                          (modifier >= 'a' && modifier <= 'f'),
                      "unknown value modifier" << DUMP(modifier)
                                               << DUMP(val_string));
    }
    return multiplier;
}

std::string chomp(std::string maybe_with_mod)
{
    ASSERT(maybe_with_mod.length() > 0);
    auto multiplier = extract_val_modifier(maybe_with_mod);
    if (multiplier > 1)
    {
        return maybe_with_mod.substr(0, maybe_with_mod.length() - 1);
    }
    return maybe_with_mod;
}
uint32_t extract_recordsize(std::string description)
{
    std::string key("recordsize=");
    auto recordsize_val = extract_val_for_key(description, key);
    if (!recordsize_val.length())
    {
        return 0;
    }
    auto multiplier = extract_val_modifier(recordsize_val);
    recordsize_val = chomp(recordsize_val);
    return std::stoi(recordsize_val) * multiplier;
}

uint32_t extract_promisc(std::string description)
{
    std::string key("promisc");
    auto options_str = extract_options_string(description);
    auto pos = options_str.find(key);
    if (pos != std::string::npos)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

uint64_t extract_latency(std::string description)
{
    std::string key("latency=");
    auto latency_val = extract_val_for_key(description, key);
    if (!latency_val.length())
    {
        return 0;
    }
    return std::stoull(latency_val);
}

size_t extract_initial_filesize(std::string description, size_t default_size)
{
    std::string key("initial_filesize=");
    auto initial_filesize_val = extract_val_for_key(description, key);
    if (!initial_filesize_val.length())
    {
        return default_size;
    }
    auto multiplier = extract_val_modifier(initial_filesize_val);
    initial_filesize_val = chomp(initial_filesize_val);
    return std::stoul(initial_filesize_val) * multiplier;
}

size_t extract_mappingsize(std::string description, size_t default_size)
{

    std::string key("mappingsize=");
    auto mappingsize_val = extract_val_for_key(description, key);
    if (!mappingsize_val.length())
    {
        return default_size;
    }
    auto multiplier = extract_val_modifier(mappingsize_val);
    mappingsize_val = chomp(mappingsize_val);
    return std::stoul(mappingsize_val) * multiplier;
}
std::string extract_filter(std::string description)
{

    std::string key("filter=");
    // TODO Do we need to deal with "quotes" here?
    auto filter = extract_val_for_key(description, key);
    return filter;
}

std::string extract_sim_exchange_type(std::string description)
{
    std::string key("exchangetype=");
    auto filter = extract_val_for_key(description, key);
    return filter;
}
bool is_anticipate(std::string description)
{
    auto opt_str = extract_options_string(description);
    return (opt_str.find("anticipate") != std::string::npos);
}

qstream_type_t from_str(std::string stream_type_desc)
{
    qstream_type_t qstream_type = qstream_type_t::undefined;

    /* readers */
    if (!stream_type_desc.compare("mmfile_r"))
    {
        qstream_type = qstream_type_t::mmap_reader;
    }
    else if (!stream_type_desc.compare("tcp_l"))
    {
        qstream_type = qstream_type_t::tcp_listener;
    }
    else if (!stream_type_desc.compare("tcp_r"))
    {
        qstream_type = qstream_type_t::tcp_reader;
    }
    else if (!stream_type_desc.compare("udp_r"))
    {
        qstream_type = qstream_type_t::udp_reader;
    }
    else if (!stream_type_desc.compare("nulltimer"))
    {
        qstream_type = qstream_type_t::nulltimer;
    }
    else if (!stream_type_desc.compare("timer"))
    {
        qstream_type = qstream_type_t::timer;
    }
    else if (!stream_type_desc.compare("cycletimer"))
    {
        qstream_type = qstream_type_t::cycletimer;
    }
    else if (!stream_type_desc.compare("gzfile_r"))
    {
        qstream_type = qstream_type_t::gzfile;
    }
    else if (!stream_type_desc.compare("exchangesim_r"))
    {
        qstream_type = qstream_type_t::exchangesim_reader;
    }
    else if (!stream_type_desc.compare("pcap_r"))
    {
        qstream_type = qstream_type_t::pcap_reader;
    }
    else if (!stream_type_desc.compare("ctp_csv_r"))
    {
        qstream_type = qstream_type_t::ctp_csv_reader;
        /* writers */
    }
    else if (!stream_type_desc.compare("mmfile_w"))
    {
        qstream_type = qstream_type_t::mmap_writer;
    }
    else if (!stream_type_desc.compare("tcp_w"))
    {
        qstream_type = qstream_type_t::tcp_writer;
    }
    else if (!stream_type_desc.compare("udp_w"))
    {
        qstream_type = qstream_type_t::udp_writer;
    }
    else if (!stream_type_desc.compare("stdout"))
    {
        qstream_type = qstream_type_t::stdout_writer;
    }
    else if (!stream_type_desc.compare("english"))
    {
        qstream_type = qstream_type_t::english_writer;
    }
    else if (!stream_type_desc.compare("null"))
    {
        qstream_type = qstream_type_t::null_writer;
    }
    else if (!stream_type_desc.compare("stats"))
    {
        qstream_type = qstream_type_t::stats_writer;
    }
    else if (!stream_type_desc.compare("exchangesim_w"))
    {
        qstream_type = qstream_type_t::exchangesim_writer;
    }
    else if (!stream_type_desc.compare("pcap_w"))
    {
        qstream_type = qstream_type_t::pcap_writer;
    }
    return qstream_type;
}

} // namespace qstream
} // namespace miye
