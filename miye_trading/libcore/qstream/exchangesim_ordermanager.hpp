/*
 * exchangesim_ordermanager.hpp
 *
 * Purpose: provide an ordermanager for the exchange sim
 * qstreams.
 */

#include "libcore/types/types.hpp"
#include "message/command_nologging.hpp"
#include "message/message.hpp"
#include <algorithm>
#include <deque>

#include "message/english.hpp"

#pragma once

namespace miye
{
namespace qstream
{

using namespace types;

typedef std::deque<message::any_order> responses_t;

constexpr static const fp_t TICK_EPSILON = 0.0000001;

struct om
{
    struct order
    {
        order(uint64_t now, const message::order_insert* o)
            : arrival_time(now), key(o->order_key), first_quote(nullptr),
              last_quote(nullptr), market_traded_volume_at_limit(0),
              market_traded_volume_through_limit(0), open_qty(o->volume),
              exec_qty(0), price(o->price), last_quote_volume(0),
              session(o->session), side(o->side),
              first_tick_after_latency(false)
        {
        }

        uint64_t arrival_time;
        order_key_t key;
        const message::quote* first_quote;
        const message::quote* last_quote;
        quantity_t market_traded_volume_at_limit;
        quantity_t market_traded_volume_through_limit;
        quantity_t open_qty;
        quantity_t exec_qty;
        price_t price;
        quantity_t last_quote_volume;
        session_t session;
        message::side_t side;
        bool first_tick_after_latency;
    };

    uint64_t latency;
    uint64_t receipt_latency;
    instrument_id_t inst_id;
    uint32_t sequence;
    fp_t tick_size;
    strategy_id_t strategy;
    bool synthesise_book;
    message::exchange_sim_type_t type;

    std::vector<order> orders;

    om(instrument_id_t inst_id, strategy_id_t strategy, uint64_t latency,
       message::exchange_sim_type_t type, bool synthesise_book, fp_t tick_size)
        : latency(latency), receipt_latency(0), inst_id(inst_id), sequence(0),
          tick_size(tick_size > fp_t(0.0) ? tick_size / fp_t(2.0)
                                          : TICK_EPSILON),
          strategy(strategy), synthesise_book(synthesise_book), type(type)
    {
    }

    om(message::sim_exchange_command& cmd)
        : latency(cmd.fill_latency), receipt_latency(cmd.receipt_latency),
          inst_id(cmd.instrument), sequence(0), tick_size(cmd.tick_size),
          strategy(cmd.strategy), synthesise_book(cmd.synthesise_book),
          type(cmd.type)
    {
    }

    void process(uint64_t now, const message::order_insert* o,
                 responses_t& output)
    {
        if (inst_id != o->instrument)
        {
            INVARIANT_MSG(false,
                          "Sim exchange handling " << inst_id
                                                   << " received an insert for "
                                                   << o->instrument);
        }
        if (find(o->order_key) != orders.end())
        {
            INVARIANT_MSG(false,
                          "Sim exchange handling dupe insert " << o->order_key);
        }
        if (orders.size() > 1)
        {
            INVARIANT_MSG(false,
                          "Sim exchange currently handles only one live order "
                          "per instrument ");
        }

        if (receipt_latency > 0)
        {
            output.emplace_back();
            ++sequence;

            message::order_receipt* receipt =
                reinterpret_cast<message::order_receipt*>(&output.back());
            receipt->type = message::msg_type_t::order_receipt;
            receipt->session = o->session;
            receipt->instrument = o->instrument;
            receipt->order_key = o->order_key;
            receipt->cl_order_key = o->cl_order_key;
            receipt->om_receive_time = now;
            receipt->om_sent_time = now + receipt_latency;
            receipt->version = message::order_receipt::msg_version;
        }

        output.emplace_back();
        ++sequence;

        message::order_booked* booked =
            reinterpret_cast<message::order_booked*>(&output.back());
        booked->account = o->account;
        booked->cl_order_key = o->cl_order_key;
        booked->duration = o->duration;
        booked->exchange_key = sequence;
        booked->flags = o->flags;
        booked->instrument = o->instrument;
        booked->order_key = o->order_key;
        booked->price = o->price;
        booked->sequence = sequence;
        booked->session = o->session;
        booked->side = o->side;
        booked->type = message::msg_type_t::order_booked;
        booked->volume = o->volume;
        booked->version = message::order_booked::msg_version;

        orders.emplace_back(now, o);
    }

    void process(uint64_t now, const message::order_cancel* o,
                 responses_t& output)
    {
        if (inst_id != o->instrument)
        {
            INVARIANT_MSG(false,
                          "Sim exchange handling " << inst_id
                                                   << " received a cancel for "
                                                   << o->instrument);
        }

        switch (type)
        {
        case message::exchange_sim_type_t::sim:
            process_sim_cancel(now, o, output);
            break;
        case message::exchange_sim_type_t::continuous:
            process_continuous_cancel(now, o, output);
            break;
        default:
            INVARIANT_FAIL("Unsupported exchange simulator type: " << type);
            break;
        }
    }

    void process_latency_command(message::sim_exchange_command& cmd)
    {
        latency = cmd.fill_latency;
        type = cmd.type;
        receipt_latency = cmd.receipt_latency;
    }

    bool process(uint64_t now, const message::quote* quote, responses_t& output)
    {
        switch (type)
        {
        case message::exchange_sim_type_t::sim:
            return process_sim(now, quote, output);
        case message::exchange_sim_type_t::continuous:
            return process_continuous(now, quote, output);
        default:
            break;
        }
        INVARIANT_FAIL("Unsupported exchange simulator type: " << type);
        return false;
    }

    inline quantity_t send_fill(responses_t& output, order* it,
                                quantity_t available_qty, price_t price)
    {
        INVARIANT_MSG(std::min(it->open_qty, available_qty),
                      "Requesting fill with zero qty/open_qty");
        output.emplace_back();
        message::order_executed* executed =
            reinterpret_cast<message::order_executed*>(&output.back());
        executed->cl_order_key = it->key;
        executed->executed_price = price;
        executed->executed_volume = std::min(it->open_qty, available_qty);
        executed->flags = 0;
        executed->instrument = inst_id;
        executed->match_id = sequence;
        executed->version = message::order_executed::msg_version;
        executed->order_key = it->key;
        executed->sequence = sequence;
        executed->session = it->session;
        executed->side = it->side;
        executed->type = message::msg_type_t::order_executed;
        ++sequence;
        return executed->executed_volume;
    }

    inline void fill(order* it, const message::quote* quote,
                     responses_t& output, quantity_t exclude_qty,
                     size_t max_depth)
    {

        size_t level = 0;
        switch (it->side)
        {
        case message::side_t::buy:
            do
            {
                fp_t ask;
                if (tick_size > fp_t(0.0))
                    ask = math::utils::to_nearest_tick(quote->ask_px[level],
                                                       tick_size);
                else
                    ask = quote->ask_px[level];

                if (ask > it->price || quote->ask_qty[level] == 0 ||
                    level > max_depth)
                    break;
                if (it->open_qty == 0)
                    break;

                quantity_t level_vol = quote->ask_qty[level];

                if (synthesise_book &&
                    quote->type == message::msg_type_t::trade_summary_quote &&
                    !quote->is_synthetic() && level == 0)
                {
                    const message::trade_summary_quote* tsq =
                        reinterpret_cast<const message::trade_summary_quote*>(
                            quote);
                    if (tsq->trade_buy_px <= it->price)
                    {
                        level_vol -= tsq->trade_buy_qty;
                    }
                }

                quantity_t quantity_available =
                    level_vol - std::min(level_vol, exclude_qty);

                exclude_qty -= std::min(exclude_qty, quantity_available);
                if (quantity_available > 0)
                {
                    quantity_t exec_qty =
                        send_fill(output, it, quantity_available, ask);
                    it->open_qty -= exec_qty;
                    it->exec_qty += exec_qty;
                }
                ++level;
            } while (true);
            break;
        case message::side_t::sell:
            do
            {
                fp_t bid;
                if (tick_size > fp_t(0.0))
                    bid = math::utils::to_nearest_tick(quote->bid_px[level],
                                                       tick_size);
                else
                    bid = quote->bid_px[level];

                if (bid < it->price || quote->bid_qty[level] == 0 ||
                    level > max_depth)
                    break;
                if (it->open_qty == 0)
                    break;

                quantity_t level_vol = quote->bid_qty[level];

                if (synthesise_book &&
                    quote->type == message::msg_type_t::trade_summary_quote &&
                    !quote->is_synthetic() && level == 0)
                {
                    const message::trade_summary_quote* tsq =
                        reinterpret_cast<const message::trade_summary_quote*>(
                            quote);
                    if (tsq->trade_sell_px >= it->price)
                    {
                        level_vol -= tsq->trade_sell_qty;
                    }
                }

                quantity_t quantity_available =
                    level_vol - std::min(level_vol, exclude_qty);

                exclude_qty -= std::min(exclude_qty, quantity_available);
                if (quantity_available > 0)
                {
                    quantity_t exec_qty =
                        send_fill(output, it, quantity_available, bid);
                    it->open_qty -= exec_qty;
                    it->exec_qty += exec_qty;
                }
                ++level;
            } while (true);
            break;
        default:
            INVARIANT_MSG(
                false,
                "Sim exchange processing an order with undefined side "
                    << it->key);
        }
    }

    inline bool process_sim(uint64_t now, const message::quote* quote,
                            responses_t& output)
    {
        /*
        We send an order to the market at t0, at t1 = t0 + latency
        1 we find a next tick after t1 (call it at t2) and check the order book
        at that time, get filled on whatever is on the order book within our
        price limit (at the price of the touch not at the order limit price) 2
        at t1 + cancel time (500ms, note not t0) we check the book again, we get
        filled for the qty LESS any partial fill from point 1 at price available
        (at the order limit price) 3 if we got a partial fill in 1, we get
        filled on any qty the trades AT or THROUGH our order limit price between
        t2 and t1 + cancel time LESS qty filled at t2 in point 1 4 if we didn’t
        get partial fill in 1, we get filled on any qty that trades THROUGH our
        order price limit between t2 and t1 + cancel time again at the order
        limit price
        */

        if (orders.size() > 0 && quote->instrument == inst_id &&
            quote->is_tradeable())
        {
            for (auto it = orders.begin(); it < orders.end(); ++it)
            {
                if (now - it->arrival_time >= latency)
                {
                    if (!it->first_quote)
                    {
                        it->first_quote = quote;
                        it->first_tick_after_latency = true;
                    }
                    else
                    {
                        it->last_quote = quote;

                        // note currently don't round prices to nearest tick for
                        // executions - maybe we should
                        if (quote->type ==
                            message::msg_type_t::trade_summary_quote)
                        {
                            // keep track of executed volume at our price or
                            // better
                            const message::trade_summary_quote* ts =
                                reinterpret_cast<
                                    const message::trade_summary_quote*>(quote);
                            switch (it->side)
                            {
                            case message::side_t::buy:
                                if (math::utils::approximatelyEqual(
                                        ts->trade_buy_px, it->price, tick_size))
                                {
                                    it->market_traded_volume_at_limit +=
                                        ts->trade_buy_qty;
                                }
                                else if (ts->trade_buy_px < it->price)
                                {
                                    it->market_traded_volume_through_limit +=
                                        ts->trade_buy_qty;
                                }
                                break;
                            case message::side_t::sell:
                                if (math::utils::approximatelyEqual(
                                        ts->trade_sell_px,
                                        it->price,
                                        tick_size))
                                {
                                    it->market_traded_volume_at_limit +=
                                        ts->trade_sell_qty;
                                }
                                else if (ts->trade_sell_px > it->price)
                                {
                                    it->market_traded_volume_through_limit +=
                                        ts->trade_sell_qty;
                                }
                                break;
                            default:
                                INVARIANT_MSG(false,
                                              "Sim exchange processing an "
                                              "order with undefined side "
                                                  << it->key);
                            }
                        }
                    }
                }
                else
                {
                    it->first_quote = quote;
                }
            }
            return true;
        }
        return false;
    }

    inline void process_sim_cancel(uint64_t now, const message::order_cancel* o,
                                   responses_t& output)
    {

        std::vector<order>::iterator it = find(o->order_key);
        if (find(o->order_key) == orders.end())
        {
            INVARIANT_MSG(false,
                          "Sim exchange cannot find for cancel order key "
                              << o->order_key);
        }

        // no tick after our latency window
        if (it->first_quote)
        {
            if (!it->last_quote)
            {
                // fill using the most recent first quote that arrived before
                // out latency window fill at price at our limit or better
                fill(&(*it),
                     it->first_quote,
                     output,
                     0,
                     message::quote::MAX_BOOK_LEVELS - 1);
            }
            else
            {
                // fill - removing already executed volume
                fill(&(*it),
                     it->first_quote,
                     output,
                     it->exec_qty,
                     message::quote::MAX_BOOK_LEVELS - 1);
                if (it->open_qty > 0)
                {
                    // consider trades that have gone through when resting on
                    // the book
                    if (it->exec_qty > 0)
                    {
                        // we had a partial fill on the first tick past our
                        // latency considers trade at or through our limit price
                        int available = it->market_traded_volume_at_limit +
                                        it->market_traded_volume_through_limit;
                        available -= it->exec_qty;
                        if (available > 0)
                        {
                            quantity_t exec_qty =
                                send_fill(output, &(*it), available, it->price);
                            it->open_qty -= exec_qty;
                            it->exec_qty += exec_qty;
                        }
                    }
                    else
                    {
                        int available = it->market_traded_volume_through_limit;
                        available -= it->exec_qty;
                        if (available > 0)
                        {
                            quantity_t exec_qty =
                                send_fill(output, &(*it), available, it->price);
                            it->open_qty -= exec_qty;
                            it->exec_qty += exec_qty;
                        }
                    }
                }
            }
        }

        if (it->open_qty > 0)
        {
            send_cancel(&(*it), o, output);
        }
        orders.erase(it);
    }

    inline bool process_continuous(uint64_t now, const message::quote* quote,
                                   responses_t& output)
    {
        if (orders.size() > 0 && quote->instrument == inst_id &&
            quote->is_tradeable())
        {
            for (auto it = orders.begin(); it < orders.end();)
            {
                if (now - it->arrival_time >= latency)
                {
                    INVARIANT_MSG(it->open_qty > 0,
                                  time::as_utc(now)
                                      << " " << (now - it->arrival_time) << " "
                                      << orders.size() << " " << it->key << " "
                                      << it->open_qty);
                    switch (it->side)
                    {
                    case message::side_t::buy:
                        if (quote->ask_px[0] <= it->price)
                        {
                            int32_t available = (int32_t)quote->ask_qty[0] -
                                                (int32_t)it->last_quote_volume;
                            if (available > 0)
                            {
                                quantity_t exec =
                                    std::min(available, (int32_t)it->open_qty);

                                output.emplace_back();
                                ++sequence;

                                message::order_executed* executed =
                                    reinterpret_cast<message::order_executed*>(
                                        &output.back());
                                executed->cl_order_key = it->key;
                                executed->executed_price = quote->ask_px[0];
                                executed->executed_volume = exec;
                                executed->flags = 0;
                                executed->instrument = inst_id;
                                executed->match_id = sequence;
                                executed->version =
                                    message::order_executed::msg_version;
                                executed->order_key = it->key;
                                executed->sequence = sequence;
                                executed->session = it->session;
                                executed->side = it->side;
                                executed->type =
                                    message::msg_type_t::order_executed;
                                it->open_qty -= exec;
                            }
                            it->last_quote_volume = quote->ask_qty[0];
                        }
                        else
                        {
                            it->last_quote_volume = 0;
                        }
                        break;
                    case message::side_t::sell:
                        if (quote->bid_px[0] >= it->price)
                        {
                            int32_t available = (int32_t)quote->bid_qty[0] -
                                                (int32_t)it->last_quote_volume;
                            if (available > 0)
                            {
                                quantity_t exec =
                                    std::min(available, (int32_t)it->open_qty);
                                output.emplace_back();
                                ++sequence;

                                message::order_executed* executed =
                                    reinterpret_cast<message::order_executed*>(
                                        &output.back());
                                executed->cl_order_key = it->key;
                                executed->executed_price = quote->bid_px[0];
                                executed->executed_volume = exec;
                                executed->flags = 0;
                                executed->instrument = inst_id;
                                executed->match_id = sequence;
                                executed->version =
                                    message::order_executed::msg_version;
                                executed->order_key = it->key;
                                executed->sequence = sequence;
                                executed->session = it->session;
                                executed->side = it->side;
                                executed->type =
                                    message::msg_type_t::order_executed;
                                it->open_qty -= exec;
                            }
                            it->last_quote_volume = quote->bid_qty[0];
                        }
                        else
                        {
                            it->last_quote_volume = 0;
                        }

                        break;
                    default:
                        INVARIANT_MSG(false,
                                      "Sim exchange processing an order with "
                                      "undefined side "
                                          << it->key);
                    }
                    if (it->open_qty == 0)
                    {
                        it = orders.erase(it);
                    }
                    else
                    {
                        ++it;
                    }
                }
                else
                {
                    ++it;
                }
            }
            return true;
        }
        return false;
    }

    inline void send_cancel(order* it, const message::order_cancel* o,
                            responses_t& output)
    {
        output.emplace_back();
        ++sequence;

        message::order_cancelled* cancelled =
            reinterpret_cast<message::order_cancelled*>(&output.back());
        cancelled->cancelled_volume = it->open_qty; // cancel everything
        cancelled->cl_order_key = o->order_key;
        cancelled->flags = 0;
        cancelled->instrument = o->instrument;
        cancelled->order_key = o->order_key;
        cancelled->reason = o->reason;
        cancelled->sequence = sequence;
        cancelled->session = o->session;
        cancelled->side = it->side;
        cancelled->type = message::msg_type_t::order_cancelled;
        cancelled->version = message::order_cancelled::msg_version;
    }

    inline void process_continuous_cancel(uint64_t now,
                                          const message::order_cancel* o,
                                          responses_t& output)
    {

        std::vector<order>::iterator it = find(o->order_key);
        if (find(o->order_key) == orders.end())
        {
            INVARIANT_MSG(false,
                          "Sim exchange cannot find for cancel order key "
                              << o->order_key);
        }
        send_cancel(&(*it), o, output);
        orders.erase(it);
    }

    inline std::vector<order>::iterator find(order_key_t key)
    {
        std::vector<order>::iterator result = std::find_if(
            orders.begin(), orders.end(), [key](order const& entry) {
                return entry.key == key;
            });
        return result;
    }
};

} // namespace qstream
} // namespace miye
