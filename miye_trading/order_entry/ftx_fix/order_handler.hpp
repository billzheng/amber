#pragma once
#include "instrument_t.hpp"
#include "libcore/qstream/mmap_writer.hpp"
#include "libcore/utils/logging.hpp"
#include "libcore/time/profiler.hpp"
#include "libcore/essential/assert.hpp"
#include <array>
#include <cassert>
#include <unordered_map>

#include "../ftx_fix/fix_enum.hpp"
#include "../ftx_fix/fix_price_utils.hpp"
#include "../ftx_fix/fix_session.hpp"
#include "../ftx_fix/order.hpp"
#include "../ftx_fix/order_container.hpp"
#include "../ftx_fix/time_utils.hpp"

namespace qx
{
namespace cme
{
using side_t = ::qx::types::message::side_t;

inline side_t to_side(SideEnum side)
{
    return side == SideEnum::Buy ? side_t::buy : side_t::sell;
}

template<typename Clock_t, typename session_t>
struct order_handler_t
{
    using mm_writer_t   = ::qx::qstream::mmap_writer<Clock_t>;
    using tcp_writer_t  = ::qx::qstream::tcp_writer<Clock_t>;
    using fix_session_t = cme::FixSession;

    using order_insert_t   = ::qx::types::message::order_insert;
    using order_cancel_t   = ::qx::types::message::order_cancel;
    using order_replace_t  = ::qx::types::message::order_replace;
    using order_executed_t = ::qx::types::message::order_executed;
    using order_booked_t   = ::qx::types::message::order_booked;
    using order_rejected_t = ::qx::types::message::order_rejected;
    using side_t           = ::qx::types::message::side_t;

    using instrument_map_t = std::unordered_map<qx::types::instrument_id_t, instrument_t>;
    using time_point_t     = std::chrono::high_resolution_clock::time_point;

    order_handler_t(instrument_map_t& instruments,
                    mm_writer_t*      mm_order_response_writer,
                    tcp_writer_t*     tcp_writer,
					mm_writer_t*	  fix_log_writer,
                    const char*       buffer,
                    const session_t&  session)
    : instruments(instruments)
    , out_buffer(buffer)
    , fix_session(session)
    , mm_order_response_writer(mm_order_response_writer)
    , tcp_writer(tcp_writer)
    , fix_log_writer(fix_log_writer)
    , sequence(0)
    {
    }

    //------------------------------------------------------------------------------------------------------//
    //-------------------------------------- outgoing message functions-------------------------------------//
    //------------------------------------------------------------------------------------------------------//
    template<typename Profiler>
    inline uint64_t place_order(const order_insert_t& oi, const uint64_t now, const std::string& timestamp, uint64_t hi_arb_start_cycles,  Profiler& prof)
    {
        // only support limit order for now
        ASSERT(oi.duration == qx::types::message::duration_t::day);
        ASSERT(oi.side != qx::types::message::side_t::undefined);
        const cme::SideEnum side =
            oi.side == qx::types::message::side_t::sell ? cme::SideEnum::Sell : cme::SideEnum::Buy;
        auto const& ins = get_security_desc(oi.instrument);

        size_t len = 0;

        double d_price = price_utils::round_by_tick_and_precision(oi.price, ins.tick_size, ins.precision);
        double price = price_utils::apply_price_factor_to_out_price(d_price, ins.display_factor);

        int32_t last_precision = qx::price_utils::get_precision_delta(ins.precision, ins.display_factor);

		len = fix_session.placeOrder(cme::OrderTypeEnum::Limitorder,
										 cme::TimeInforceEnum::Day,
										 oi.order_key.key,
										 side,
										 price,			// double
										 last_precision,
										 oi.volume,
										 ins.symbol,
										 ins.security_desc,
										 timestamp);
        TIME(fix_msg_prepared);

        auto send_ts = send_and_log(out_buffer, len);
        TIME(fix_msg_sent);

        WRITE_TIME(prof, time::profiler_trigger::om_fix_msg_prep, fix_msg_prepared - hi_arb_start_cycles, 0, 0, 0, 0, 0);
        WRITE_TIME(prof, time::profiler_trigger::om_msg_sent, fix_msg_sent - fix_msg_prepared, 0, 0, 0, 0, 0);

        // send order receipt back to trading engine for profile measurement
        {
        	types::message::order_receipt om {};
        	om.order_key = oi.order_key;
        	om.instrument = oi.instrument;
        	om.session = trading::session;
        	/*
        	 * if md_key is populated, sets to om_receive_time for profiling
        	 */
        	om.om_receive_time = oi.md_key > 0 ? oi.md_key.key : now;
        	om.om_sent_time = send_ts;
        	mm_order_response_writer->write(&om, sizeof(types::message::order_receipt));
        }

        // map order_insert_t.order_key.key to clord_id(tag 11)
        // when order acks from exchange, set order_t::order_id to exchange_order_id(tag37)
        const order_id_t clord_id = oi.order_key.key;
        orders.add(clord_id, order_t(oi, fix_session.getLastSequenceNum()));

        LOG_INFO("[place_order] instrument: cme_inst: " << ins.instrument_id
        		  << ", order_id: " << oi.order_key.key
        		  << ", cl_order_id: " << oi.cl_order_key.key
				  << ", oi.order: " << oi.order_key
        		  << ", instrument: " << oi.instrument
				  << ", order_type: " << toUnderlyingType(cme::OrderTypeEnum::Limitorder)
				  << ", tif: " << toUnderlyingType(cme::TimeInforceEnum::Day)
        		  << ", original price: " << oi.price
				  << ", applied display factor price: " << price
				  << ", qty: " << oi.volume
				  << ", side: " << oi.side
				  << ", symbol: " << ins.symbol
				  << ", security_desc: " << ins.security_desc
				  << ", send_ts: " << send_ts);
        return send_ts;
    }

    inline uint64_t cancel_order(const order_cancel_t& o, const uint64_t now, const std::string& timestamp)
    {
        const uint64_t order_id = o.order_key.key;
        auto const&    ins      = get_security_desc(o.instrument);

        assert(o.side != side_t::undefined);
        const cme::SideEnum side = o.side == side_t::sell ? cme::SideEnum::Sell : cme::SideEnum::Buy;

        auto* order = orders.get(order_id);
        if (!order)
        {
        	LOG_INFO("[cancel_order] order not found, order_id: " << order_id);
            return 0;
        }
        else
        {
            // just return if orders are not found?
            // how should we handle if order entry adapter is restarted?
            auto const len = fix_session.cancelOrder(side, order->exchange_key.key, order_id, ins.symbol, ins.security_desc, timestamp);
            auto send_ts = send_and_log(out_buffer, len);
            order->last_request = types::message::msg_type_t::order_cancel;
            LOG_INFO("[cancel_order] cancel order: " << *order
				  << ", receive_time: " << now
				  << ", send_ts: " << send_ts);
            return send_ts;
        }
    }

    uint64_t send_and_log(const char* buf, size_t len) const
    {
        ASSERT_MSG(len <= FixSession::OutBufferSize, DUMP(len) << DUMP(FixSession::OutBufferSize));
        tcp_writer->write(buf, len);
        auto ts = tcp_writer->last_send_ts();
        fix_log_writer->write(buf, len);
        return ts;
    }

    void logon()
    {
    	auto const len = fix_session.logon();
    	send_and_log(out_buffer, len);
    }

    void logout()
    {
    	if (session_enabled())
    	{
			auto const len = fix_session.logout();
			send_and_log(out_buffer, len);
    	}
    }

    void send_heartbeat()
    {
    	auto const len = fix_session.sendHeartBeat();
    	send_and_log(out_buffer, len);
    }

    void log(const char* buf, size_t len) const
    {
    	fix_log_writer->write(buf, len);
    }
    /*
     * not supported at the moment
     */
    inline void cancel_replace_order(tcp_writer_t&, fix_session_t&, const order_replace_t&, const std::string&)
    {
    	assert(0);
    }

    //------------------------------------------------------------------------------------------------------//
    //-------------------------------------- incoming message callbacks ------------------------------------//
    //------------------------------------------------------------------------------------------------------//
    void consume_tcp_buffer(const ::qx::qstream::place& plc)
    {

        fix_session.consume_tcp_buffer(*this, plc, session_enabled());
    }

    void onOrderAck(uint64_t     exchange_instrument_id,
					order_id_t   order_id,
					clorder_id_t clord_id,
                    quantity_t   qty,
                    price_t      price,
                    SideEnum     side,
					const std::string& security_desc)
    {

        assert(side != SideEnum::Unknown);

        ::qx::types::message::order_booked om{};
        om.session = trading::session;
        om.order_key.key = clord_id;
        om.volume = qty;

        om.price = price_utils::apply_price_factor_to_in_price(price, get_display_factor(security_desc));

        om.exchange_key = order_id;
        om.side = to_side(side);
        om.sequence = sequence;

        auto* order = orders.get(clord_id);
        if (order)
        {
            order->status = order_status_t::booked;
            order->exchange_key.key = order_id;
            om.cl_order_key = order->clord_id;
            om.instrument = order->instrument;
            om.duration = order->duration;

            LOG_INFO("onOrderAck, change order exchange id and status: " << *order);
        }
        else
        {
            uint32_t qcl_instrument_id = get_qcl_instrument(security_desc);
            om.instrument = qcl_instrument_id;

            LOG_ERROR("Unable to locate exchange order on order ack " << order_id << " for exchange instrument " << exchange_instrument_id << " qcl_instrument_id " << qcl_instrument_id);
        }

        mm_order_response_writer->write(&om, sizeof(order_booked_t));
        ++sequence;

        LOG_INFO("onOrderAck, exchange_instrument_id: " << exchange_instrument_id
                << ", instrument_id: " << om.instrument
        		<< ", security_desc: "   << security_desc
        		<< ", clord_id: " << clord_id
        		<< ", order_id: " << order_id
				<< ", qty: "      << qty
				<< ", price: "    << price
				<< ", side: "     << static_cast<char>(side));

    }

    void onOrderReject(uint64_t           exchange_instrument_id,
    				   order_id_t         order_id,
                       clorder_id_t       clord_id,
                       quantity_t         qty,
                       uint32_t           reject_reason,
					   const std::string& security_desc,
                       const std::string& text)
    {
    	using qx::types::message::reject_reason_t;

        order_rejected_t om{};
        om.session          = trading::session;
        om.order_key.key    = clord_id;
        om.rejected_volume  = qty;
        om.reason_text      = text;
        om.sequence = sequence;

        auto* order = orders.get(clord_id);
        if (order)
        {
            om.instrument       = order->instrument;
            om.cl_order_key = order->clord_id;
            om.request_type = order->last_request;

            auto const cnt = orders.remove(clord_id);
            LOG_INFO("[onOrderReject] order rejected by gateway, exchange order_id: " << order_id
                    << ", clord_id: " << clord_id
                    << ", qty: " << qty
                    << ", reject_id: " << reject_reason
                    << ", security_desc: " << security_desc
                    << ", reject reason: " << text
                    << " " << cnt << " order removed from cache");
        }
        else
        {
            uint32_t qcl_instrument_id = get_qcl_instrument(security_desc);
            om.instrument = qcl_instrument_id;
            LOG_ERROR("Unable to locate exchange order on reject " << order_id << " for exchange instrument " << exchange_instrument_id << " qcl_instrument_id " << qcl_instrument_id);
        }


        /*
         * http://www.cmegroup.com/confluence/display/EPICSANDBOX/iLink+Reject+Codes
         */
        if (reject_reason == 2130 || reject_reason == 1003)
        {
        	om.reason = reject_reason_t::closed;
        }
        else
        {
        	om.reason = reject_reason_t::parameter;
        }
        mm_order_response_writer->write(&om, sizeof(order_rejected_t));
        ++sequence;

    }

    void onFill(uint64_t           exchange_instrument_id,
                uint64_t           order_id,
                uint64_t           clord_id,
                price_t            price,
                uint32_t           exec_qty,
                uint32_t           leaves_qty,
                SideEnum           side,
                const std::string& security_desc,
                bool               full_fill)
    {
        order_executed_t om {};

        om.session          = trading::session;
        om.order_key.key    = clord_id;

        double display_factor = get_display_factor(security_desc);

        om.executed_price  = price_utils::apply_price_factor_to_in_price(price, display_factor);
        om.executed_volume = exec_qty;
        om.side            = to_side(side);
        om.sequence        = sequence;
        ++sequence;

        LOG_INFO("[onFill], order fill, order_id: " << order_id
        		<< ", clord_id: " << clord_id
				<< ", price: " << price
				<< ", real price: " << om.executed_price
				<< ", volume: " << exec_qty
				<< ", leaves_qty: " << leaves_qty
				<< ", side: " << toUnderlyingType(side)
				<< ", security_desc: " << security_desc
				<< ", is_full_fill: " << full_fill);

        auto* order = orders.get(clord_id);

        if (order)
        {
            om.instrument = order->instrument;
            om.cl_order_key = order->clord_id;
        }
        else
        {
            uint32_t qcl_instrument_id = get_qcl_instrument(security_desc);
            om.instrument = qcl_instrument_id;
            LOG_ERROR("Unable to locate exchange order on fill " << order_id << " for exchange instrument " << exchange_instrument_id << " qcl_instrument_id " << qcl_instrument_id);
        }

        mm_order_response_writer->write(&om, sizeof(order_executed_t));

        if (full_fill)
        {
            auto const cnt = orders.remove(clord_id);
            LOG_INFO("[onFill] " << cnt << " order removed from cache, clord_id: " << clord_id);
        }
    }

    void onCancelAck(uint64_t     exchange_instrument_id,
                     order_id_t   order_id,
                     clorder_id_t clord_id,
                     quantity_t   qty,
                     price_t      price,
                     SideEnum     side,
					 const std::string& security_desc)
    {
        ::qx::types::message::order_cancelled om{};

        om.session          = trading::session;
        om.order_key.key    = clord_id;

        om.cancelled_volume = qty;
        om.side             = to_side(side);
        om.sequence = sequence++;

        auto* order = orders.get(clord_id);
        if (order)
        {
            om.instrument = order->instrument;
            om.cl_order_key = order->clord_id;
        }
        else
        {
            uint32_t qcl_instrument_id = get_qcl_instrument(security_desc);
            om.instrument = qcl_instrument_id;
            LOG_ERROR("Unable to locate exchange order cancel ack " << order_id << " for exchange instrument " << exchange_instrument_id << " qcl_instrument_id " << qcl_instrument_id);
        }
        mm_order_response_writer->write(&om, sizeof(::qx::types::message::order_cancelled));

        auto const cnt = orders.remove(clord_id);

        LOG_INFO("[onCancelAck] order_id: " << order_id
        		<< ", clord_id: " << clord_id
				<< ", price: " << price
				<< ", volume: " << qty
				<< ", side: " << toUnderlyingType(side)
				<< " " << cnt << " order removed from cache");
    }

    void onOrderCancelReject(uint64_t order_id, uint64_t clord_id, const std::string& text)
    {
    	::qx::types::message::order_rejected om {};
        om.order_key.key    = clord_id;
        om.session          = trading::session;
        om.reason_text      = text;
        auto* order = orders.get(clord_id);
        if (order)
        {
            om.instrument = order->instrument;
            om.cl_order_key = order->clord_id;
        }
        else
        {
            LOG_INFO("Unable to locate exchange order cancel reject " << order_id);
        }

        mm_order_response_writer->write(&om, sizeof(::qx::types::message::order_rejected));

    	LOG_INFO("[onOrderCancelReject] order_id: " << order_id
    			<< ", clord_id: " << clord_id
				<< ", reason: " << text);
    }

    void onTradeCancelled(uint64_t           order_id,
                          uint64_t           clord_id,
                          price_t            price,
                          uint32_t           last_qty,
                          SideEnum           side,
                          const std::string& exec_id,
                          const std::string& symbol)
    {
        LOG_ERROR("trade cancelled, symbol:" << symbol
        		  << ", order_id: " << order_id
        		  << ", clord_id: " << clord_id
				  << ", price: "    << price
				  << ", last_qty: "	<< last_qty
				  << ", side: "		<< toUnderlyingType(side)
				  << ", trade_id: " << exec_id);
    }

    void onSessionLevelReject(uint32_t seq, const std::string& reason)
    {
        const order_t* o = orders.getBySequence(seq);
        if (o)
        {
            onOrderCancelReject(o->order_id.key, o->clord_id.key, reason);
        }
        else
        {
            LOG_ERROR("Unable to find order matching fix sequence " << seq);
        }
    }

    void on_logon()
    {
    	enable_session();
    }

    void on_logout()
    {
    	disable_session();
    }

    const cme::instrument_t& get_security_desc(qx::types::instrument_id_t instrument_id) const
    {
        auto it = instruments.find(instrument_id);
        assert(it != instruments.end());
        return it->second;
    }

    uint32_t get_qcl_instrument(const std::string& security_desc) const
    {
        for (auto const e : instruments)
        {
            if (e.second.security_desc == security_desc)
            {
                return e.first;
            }
        }

        return 0;
    }

    double get_display_factor(const std::string& security_desc) const
    {
        for (auto const e : instruments)
        {
            auto const& instrument = e.second;
            if (instrument.security_desc == security_desc)
            {
                return instrument.display_factor;
            }
        }

        LOG_WARNING("display factor not found for " << security_desc << " use default 1.0");
        return 1.0;
    }

    session_t& get_session() { return fix_session; }

    const time_point_t& get_last_sent_tp() const { return last_sent_tp; }
    void set_last_sent_tp(const time_point_t& tp) { last_sent_tp = tp; }

    bool can_send_next_msg(const time_point_t& tp) const
    {
    	return tp - last_sent_tp > std::chrono::seconds(29);
    }
    void disable_session() { is_session_ok = false; }
    void enable_session() { is_session_ok = true; }
    bool session_enabled() const { return is_session_ok; }
    void warm_cache()
    {
        if(tcp_writer){
            // this causes untraceable onload segfault
            //PREFETCH_WRITE(&tcp_writer->writebuffer[0]);
        }
    }

private:
    instrument_map_t& instruments;
    const char*       out_buffer{};
    session_t         fix_session{};
    mm_writer_t*      mm_order_response_writer{};
    tcp_writer_t*     tcp_writer{};
    mm_writer_t*	  fix_log_writer {};
    time_point_t      last_sent_tp;
    bool is_session_ok {};
    order_container   orders;
    uint32_t          sequence;
};

} // namespace cme
} // namespace qx

