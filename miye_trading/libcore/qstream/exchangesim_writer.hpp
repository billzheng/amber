/*
 * exchangesim_writer.hpp
 *
 * Purpose: Simulate an exchange
 */

#include "libcore/essential/assert.hpp"
#include "libcore/qstream/exchangesim_ordermanager.hpp"
#include "libcore/qstream/qstream_writer_interface.hpp"
#include "libcore/time/timeutils.hpp"
#include "message/command_nologging.hpp"
#include "message/english.hpp"
#include "message/message.hpp"
#include "qstream_common.hpp"

#pragma once

namespace miye
{
namespace qstream
{

using namespace types;

template <typename Clock>
class exchangesim_writer
    : public qstream_writer_interface<exchangesim_writer<Clock>>
{
  public:
    exchangesim_writer(Clock& clk_, const std::string& description_)
        : clk(clk_), description(description_), stateflags(0), insert_time(0)
    {
        latency = extract_latency(description);
        type = message::from_str<message::exchange_sim_type_t>(
            qstream::extract_sim_exchange_type(description).c_str());
        synthetic_book = description.find("synthetic") != std::string::npos;
        tick_size = fp_t(0.0);
    }

    place pledge(size_t n)
    {
        INVARIANT_MSG(
            n <= message::any_order::msg_size,
            "Exchange simulator pledge request for more than order_msg size ["
                << n << "]");
        return place(&incoming, n);
    }

    int announce(const place& descriptor)
    {
        uint64_t now = clk.now();
        const message::miye_msg* m =
            reinterpret_cast<const message::miye_msg*>(descriptor.start);
        switch (m->type)
        {
        case message::msg_type_t::order_insert: {
            const message::order_insert* oi =
                reinterpret_cast<const message::order_insert*>(
                    descriptor.start);
            find(oi->instrument, oi->order_key, true)
                ->process(now, oi, order_responses);
            break;
        }
        case message::msg_type_t::order_cancel: {
            const message::order_cancel* oc =
                reinterpret_cast<const message::order_cancel*>(
                    descriptor.start);
            find(oc->instrument, oc->order_key, true)
                ->process(now, oc, order_responses);
            break;
        }
        case message::msg_type_t::quote:
        case message::msg_type_t::trade_summary_quote: {
            const message::quote* q =
                reinterpret_cast<const message::quote*>(descriptor.start);
            for (auto& o : order_managers)
            {
                if (o.inst_id == q->instrument)
                {
                    o.process(now, q, order_responses);
                }
            }
            break;
        }
        case message::msg_type_t::command: {
            const message::command* c =
                reinterpret_cast<const message::command*>(descriptor.start);
            if (c->command_type == message::command_t::sim_exchange_command)
            {
                message::sim_exchange_command cmd;
                parsing::parse_json(cmd, c->to_str());
                for (auto& o : order_managers)
                {
                    if (o.inst_id == cmd.instrument &&
                        o.strategy == cmd.strategy)
                    {
                        o.process_latency_command(cmd);
                        return 0; // success
                    }
                }
                order_managers.emplace_back(cmd);
            }
            break;
        }
        default:
            INVARIANT_MSG(false,
                          "Exchange simulator only handles order_insert and  "
                          "order_cancel, received "
                              << m->type);
            break;
        }
        return 0; // success
    }

    void process(const message::quote* quote)
    {
        uint64_t now = this->clk.now();
        for (auto& o : order_managers)
        {
            if (o.process(now, quote, order_responses))
            {
                break;
            }
        }
    }

    uint64_t state() const
    {
        return stateflags;
    }

  private:
    inline om* find(instrument_id_t instrument, order_key_t order_key,
                    bool create)
    {
        auto it = std::find_if(order_managers.begin(),
                               order_managers.end(),
                               [instrument, order_key](om const& entry) {
                                   return entry.inst_id == instrument &&
                                          entry.strategy == order_key.strategy;
                               });
        if (it == order_managers.end())
        {
            if (create)
            {
                order_managers.emplace_back(instrument,
                                            order_key.strategy,
                                            latency,
                                            type,
                                            synthetic_book,
                                            tick_size);
                return &order_managers.back();
            }
            else
            {
                return nullptr;
            }
        }
        else
        {
            return &(*it);
        }
    }

  public:
    Clock& clk;
    const std::string description;
    uint64_t stateflags;
    uint64_t insert_time;
    uint64_t latency;
    message::any_order incoming;
    std::vector<om> order_managers;
    responses_t order_responses;
    message::exchange_sim_type_t type;
    bool synthetic_book;
    fp_t tick_size;
};

} // namespace qstream
} // namespace miye
