/*
 * stats.hpp
 *
 * Purpose: print out stats on the stream
 * Author
 *
 */

#pragma once

#include <cmath>
#include <limits>
#include <map>
#include <regex>
#include <vector>

#include "libcore/essential/assert.hpp"
#include "libcore/qstream/mmap_headers.hpp"
#include "libcore/qstream/qstream_writer_interface.hpp"
#include "libcore/time/clock.hpp"
#include "libcore/time/timeutils.hpp"
#include "message/message.hpp"
#include "trading/adapter/cme/md/mdp3/AnalysisMessageReaders.hpp"
#include "trading/engine/md_analysis.hpp"

using std::vector;

namespace miye
{
namespace qstream
{

template <typename Clock>
class stats_writer : public qstream_writer_interface<stats_writer<Clock>>
{
  public:
    stats_writer(Clock& clk_, const std::string& description_)
        : clk(clk_), description(description_), record_count_(0),
          total_bytes_written_(0),
          smallest_record_size_(std::numeric_limits<uint64_t>::max()),
          largest_record_size_(0), first_time_(0), last_time_(0),
          collect_packet_timing_(false), pcap_info_(false),
          bin_width_(time::millis(1)), previous_time_(0),
          min_interval_(std::numeric_limits<uint64_t>::max()),
          last_sequence_(0), cme_time(nullptr), cme_seq(nullptr),
          cme_drift(nullptr), md_analysis(nullptr)
    {
        if (description.find("packet_timing") != std::string::npos)
        {
            collect_packet_timing_ = true;
            size_t pos = description.find("packet_timing=");
            if (pos != std::string::npos)
            {
                // we have a bin width specified
                bin_width_ =
                    time::parse_nano_interval(description.substr(pos + 14));
            }
        }
        if (description.find("pcap") != std::string::npos)
        {
            pcap_info_ = true;

            bool histogram = description.find("hist") != std::string::npos;

            if (description.find("cmetime") != std::string::npos)
            {
                uint32_t ticks = 0;
                std::regex r0("ticks=(\\d+)");
                std::smatch result0;
                if (std::regex_match(
                        description.begin(), description.end(), result0, r0))
                {
                    ticks = std::stoi(result0[1]);
                }

                uint32_t inst_id = 0;
                std::regex r1("inst=(\\d+)");
                std::smatch result1;
                if (std::regex_search(
                        description.begin(), description.end(), result1, r1))
                {
                    inst_id = std::stoi(result1[1]);
                }

                cme_time.reset(new trading::cme::mdp3_time_delta_handler());
                cme_time.get()->init(histogram, ticks, inst_id);
            }
            if (description.find("cmeseq") != std::string::npos)
            {
                cme_seq.reset(new trading::cme::mdp3_pcap_sequence_handler(
                    (description.find("verbose") != std::string::npos)));
            }
            if (description.find("cmedrift") != std::string::npos)
            {

                uint32_t bucket = 0;
                std::regex r0("bucket=(\\d+)");
                std::smatch result0;
                if (std::regex_search(
                        description.begin(), description.end(), result0, r0))
                {
                    bucket = std::stoi(result0[1]);
                }
                cme_drift.reset(new trading::cme::mdp3_machine_drift());
                cme_drift.get()->init(histogram, bucket);
            }
        }
        if (description.find("md") != std::string::npos)
        {
            md_analysis.reset(new trading::md_analysis());
        }
    }

    ~stats_writer()
    {
        std::cout << "      Message Stats:" << std::endl;
        std::cout << "            Records: " << record_count_ << std::endl;
        std::cout << "              Bytes: " << total_bytes_written_
                  << std::endl;
        std::cout << "           Smallest: " << smallest_record_size_
                  << std::endl;
        std::cout << "            Largest: " << largest_record_size_
                  << std::endl;
        std::cout << "              First: " << time::as_utc(first_time_)
                  << std::endl;
        std::cout << "               Last: " << time::as_utc(last_time_)
                  << std::endl;
        std::cout << "       Min interval: " << min_interval_ << std::endl;

        if (collect_packet_timing_)
        {
            std::cout << "Inter packet timing distribution:" << std::endl;
            for (size_t i = 0; i < timing_bins_.size(); ++i)
            {
                if (timing_bins_[i] > 0)
                {
                    std::cout << time::format_nano_interval(i * bin_width_)
                              << " : " << timing_bins_[i] << std::endl;
                }
            }
        }
        if (pcap_info_)
        {
            std::cout << "UDP statistics\n\naddress,count\n";

            for (auto it = pkt_counts.begin(); it != pkt_counts.end(); ++it)
            {
                std::cout << it->first << "," << it->second << "\n";
            }
            std::cout << "\n\n";
        }
        if (cme_seq)
        {
            std::cout << "CME message sequence statistics:" << std::endl;
            std::cout << cme_seq->to_str() << std::endl;
        }
        if (cme_time)
        {
            std::cout << "CME message time statistics:" << std::endl;
            std::cout << cme_time->to_str() << std::endl;
        }
        if (cme_drift)
        {
            std::cout << "CME machine time drift statistics:" << std::endl;
            std::cout << cme_drift->to_str() << std::endl;
        }
        if (md_analysis)
        {
            std::cout << "MD statistics:" << std::endl;
            std::cout << md_analysis->to_str() << std::endl;
        }
    }

    place pledge(size_t n)
    {
        ASSERT(n <= qstream::mmap_max_recordsize);
        return place(wbuf, n);
    }

    int announce(const place& descriptor)
    {
        if (record_count_ == 0)
        {
            first_time_ = clk.now();
        }
        previous_time_ = last_time_;
        min_interval_ = std::min(min_interval_, clk.now() - last_time_);
        last_time_ = clk.now();
        ++record_count_;
        total_bytes_written_ += descriptor.size;
        if (descriptor.size < smallest_record_size_)
        {
            smallest_record_size_ = descriptor.size;
        }
        if (descriptor.size > largest_record_size_)
        {
            largest_record_size_ = descriptor.size;
        }
        if (collect_packet_timing_ && previous_time_ != 0)
        {
            uint64_t time_since_last_write = last_time_ - previous_time_;
            size_t bin_index = time_since_last_write / bin_width_;
            if (timing_bins_.size() < bin_index)
            {
                timing_bins_.resize(bin_index + 1, 0);
            }
            ++timing_bins_[bin_index];
        }
        if (pcap_info_)
        {
            const types::message::wrapper* m =
                reinterpret_cast<const types::message::wrapper*>(
                    descriptor.start);

            qstream::place pcap_pkt = descriptor.pop_front(m->msg_header_size);
            qstream::place ip_pkt = pcap_pkt.pop_front(14);
            const iphdr* ip = (const iphdr*)ip_pkt.start;
            if (ip->version == 4)
            {
                if (ip->protocol == IPPROTO_UDP)
                {
                    qstream::place udp_pkt = ip_pkt.pop_front(sizeof(iphdr));
                    const udphdr* udp = (const udphdr*)udp_pkt.start;
                    struct in_addr addr;
                    addr.s_addr = ip->daddr;
                    char* dot_ip = inet_ntoa(addr);
                    std::string ip_address(dot_ip);
                    std::ostringstream oss;
                    oss << ip_address << ":" << htons(udp->dest);
                    auto it = pkt_counts.find(oss.str());
                    if (it == pkt_counts.end())
                    {
                        pkt_counts.insert(std::make_pair(oss.str(), 1));
                    }
                    else
                    {
                        it->second++;
                    }

                    if (cme_time || cme_seq || cme_drift)
                    {
                        qstream::place sbe_pkt =
                            udp_pkt.pop_front(sizeof(udphdr));
                        UNUSED(sbe_pkt);
                        if (cme_time)
                        {
                            cme_time.get()->process_message(clk.now(), m);
                        }
                        if (cme_seq)
                        {
                            cme_seq.get()->process_message(clk.now(), m);
                        }
                        if (cme_drift)
                        {
                            cme_drift.get()->process_message(clk.now(), m);
                        }
                    }
                }
            }
        }
        if (md_analysis)
        {
            md_analysis.get()->process_message(clk.now(), descriptor.start);
        }

        return 0; // success
    }

    Clock& clk;
    const std::string description;

  private:
    char wbuf[mmap_max_recordsize];
    size_t record_count_;
    size_t total_bytes_written_;
    size_t smallest_record_size_;
    size_t largest_record_size_;
    uint64_t first_time_;
    uint64_t last_time_;

    bool collect_packet_timing_;
    bool pcap_info_;
    uint64_t bin_width_;
    uint64_t previous_time_;
    uint64_t min_interval_;
    uint32_t last_sequence_;

    vector<size_t> timing_bins_;
    std::map<std::string, uint64_t> pkt_counts;

    std::shared_ptr<trading::cme::mdp3_time_delta_handler> cme_time;
    std::shared_ptr<trading::cme::mdp3_pcap_sequence_handler> cme_seq;
    std::shared_ptr<trading::cme::mdp3_machine_drift> cme_drift;
    std::shared_ptr<trading::md_analysis> md_analysis;
};

} // namespace qstream
} // namespace miye
