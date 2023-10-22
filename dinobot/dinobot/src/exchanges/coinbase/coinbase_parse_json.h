#ifndef _COINBASE_EXCHANGE_PARSE_JSON_MESSAGES_H
#define _COINBASE_EXCHANGE_PARSE_JSON_MESSAGES_H

#include <vector>
#include <chrono>
#include <ostream>
#include <iostream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "../../libs/json/fast/sajson.h"
#pragma GCC diagnostic pop

#include "coinbase_messages.h"

namespace exchange { namespace coinbase { namespace websocket {

    std::chrono::system_clock::time_point parse_date(std::string ss)
    {
        using namespace std::chrono;
        using namespace date;

        std::stringstream datestr(ss);

#ifdef __APPLE__
        date::sys_time<std::chrono::microseconds> tp;
#else
        date::sys_time<std::chrono::nanoseconds> tp; // mac doesn't seems to have nanos currently 
#endif

        datestr >> date::parse("%FT%T", tp);
        if (datestr.fail())
            std::cout << "Could not parse date time string, exiting " << std::endl;
        return tp;

    }

    const sajson::value is_valid(const sajson::document& doc)
    {
        if (!doc.is_valid())
        {
            std::cerr << "XX: " << doc.get_error_message_as_cstring() << std::endl;
            throw std::runtime_error("We have received an invalid JSON payload, exiting");
        }
        
        // all json payloads we receive should be an OBJECT type, so lets test for it
        const sajson::value root = doc.get_root();
        if (root.get_type() != sajson::TYPE_OBJECT)
        {
            throw std::runtime_error("We have received an invalid JSON payload, exiting");
        }
        return root;
    }

    // Heartbeat message - let's ignore for now
    // {"type": "heartbeat","sequence": 90,"last_trade_id": 20,"product_id": "BTC-USD","time": "2014-11-07T08:19:28.464459Z"}
    void parse_json_heartbeat(const sajson::value &msg, exchange::coinbase::ws_heartbeat &hb)
    {
        hb.receive_time = std::chrono::system_clock::now();;
        msg.get_value_of_key(sajson::string("sequence",8)).get_int53_value(&hb.sequence);
        msg.get_value_of_key(sajson::string("last_trade_id",13)).get_int53_value(&hb.last_trade_id);
        hb.product_id = exchange::coinbase::convert_product_id(msg.get_value_of_key(sajson::string("product_id", 10)).as_string());
        hb.time = parse_date(msg.get_value_of_key(sajson::string("time", 4)).as_string());
    }

    // error meesage
    void parse_json_error(const sajson::value &msg)
    { // TODO parse later
        (void)msg;

    }

    // Subscription respose - Let's ignore for now 
    //{"type": "subscriptions","channels": [{"name": "level2","product_ids": ["ETH-USD","ETH-EUR"],},{"name": "heartbeat","product_ids": ["ETH-USD","ETH-EUR"],},{"name": "ticker","product_ids": ["ETH-USD","ETH-EUR","ETH-BTC"]}]}
    void parse_json_subscriptions(const sajson::value &msg)
    { // TODO parse later 
        (void) msg;
    }

    // Ticker feed messages 
    // {"type": "ticker","trade_id": 20153558,"sequence": 3262786978,"time": "2017-09-02T17:05:49.250000Z","product_id": "BTC-USD","price": "4388.01000000","side": "buy","last_size": "0.03000000","best_bid": "4388","best_ask": "4388.01"}
    void parse_json_ticker(const sajson::value &msg, exchange::coinbase::ws_ticker &tick)
    {
        tick.receive_time = std::chrono::system_clock::now();
        msg.get_value_of_key(sajson::string("trade_id",8)).get_int53_value(&tick.trade_id);
        msg.get_value_of_key(sajson::string("sequence",8)).get_int53_value(&tick.sequence);
        tick.time = parse_date(msg.get_value_of_key(sajson::string("time", 4)).as_string());
        tick.product_id = exchange::coinbase::convert_product_id(msg.get_value_of_key(sajson::string("product_id", 10)).as_string());
        tick.price = std::stod(msg.get_value_of_key(sajson::string("price",5)).as_string());
        tick.side = exchange::coinbase::convert_side_type(msg.get_value_of_key(sajson::string("side", 4)).as_string());
        tick.last_size = std::stod(msg.get_value_of_key(sajson::string("last_size",9)).as_string());
        tick.best_bid = std::stod(msg.get_value_of_key(sajson::string("best_bid",8)).as_string());
        tick.best_ask = std::stod(msg.get_value_of_key(sajson::string("best_ask",8)).as_string());
    }

    //{"type": "l2update","product_id": "BTC-EUR","changes": [["buy", "6500.09", "0.84702376"],["sell", "6507.00", "1.88933140"],["sell", "6505.54", "1.12386524"],["sell", "6504.38", "0"]]}
    void parse_json_l2update(const sajson::value &msg, exchange::coinbase::ws_l2_update &update)
    {
        update.receive_time = std::chrono::system_clock::now();
        update.product_id = exchange::coinbase::convert_product_id(msg.get_value_of_key(sajson::string("product_id", 10)).as_string());   

        // loop through the snapshots and build up changes vectors
        auto changes = msg.get_value_of_key(sajson::string("changes",7));
        auto length = changes.get_length();
        for (auto i = 0u ; i < length ; ++i)
        {
            auto book_update  = changes.get_array_element(i);
            exchange::coinbase::ws_l2_update_book book;
            book.side = exchange::coinbase::convert_side_type(book_update.get_array_element(0).as_string());
            book.price = std::stod(book_update.get_array_element(1).as_string());
            book.size = std::stod(book_update.get_array_element(2).as_string());
            update.changes.push_back(std::move(book));
        }
    }
    
    //l2 snapshot packet
    // {"type": "snapshot","product_id": "BTC-EUR","bids": [["6500.11", "0.45054140"]],"asks": [["6500.15", "0.57753524"]]}
    void parse_json_l2snapshot(const sajson::value &msg, exchange::coinbase::ws_l2_snapshot &snapshot)
    {
        snapshot.receive_time = std::chrono::system_clock::now();
        snapshot.product_id = exchange::coinbase::convert_product_id(msg.get_value_of_key(sajson::string("product_id", 10)).as_string());

        // bids
        auto bids = msg.get_value_of_key(sajson::string("bids",4));
        auto bid_len = bids.get_length();
        for (auto i = 0u ; i < bid_len ; ++i)
        {
            auto bid_update = bids.get_array_element(i);
            exchange::coinbase::ws_l2_snapshot_book book;    
            book.size = std::stod(bid_update.get_array_element(0).as_string());
            book.price = std::stod(bid_update.get_array_element(1).as_string());
            snapshot.bids.push_back(std::move(book));
        }

        // asks 
        auto asks = msg.get_value_of_key(sajson::string("asks",4));
        auto ask_len = asks.get_length();
        for (auto i = 0u ; i < ask_len ; ++i)
        {
            auto ask_update = asks.get_array_element(i);
            exchange::coinbase::ws_l2_snapshot_book book;    
            book.size = std::stod(ask_update.get_array_element(0).as_string());
            book.price = std::stod(ask_update.get_array_element(1).as_string());
            snapshot.asks.push_back(std::move(book));
        }
    }

    // {"type": "received","time": "2014-11-07T08:19:27.028459Z","product_id": "BTC-USD","sequence": 10,"order_id": "d50ec984-77a8-460a-b958-66f114b0de9b","size": "1.34","price": "502.1","side": "buy","order_type": "limit"}
    void parse_json_full_received(const sajson::value &msg, exchange::coinbase::ws_full_channel_received_limit &rec)
    {
        rec.receive_time = std::chrono::system_clock::now();
        rec.product_id = exchange::coinbase::convert_product_id(msg.get_value_of_key(sajson::string("product_id", 10)).as_string());    
        rec.time = parse_date(msg.get_value_of_key(sajson::string("time", 4)).as_string());
        msg.get_value_of_key(sajson::string("sequence",8)).get_int53_value(&rec.sequence);
        strcpy(rec.order_id, msg.get_value_of_key(sajson::string("order_id",8)).as_cstring());
        rec.size = std::stod(msg.get_value_of_key(sajson::string("size",4)).as_string());     
        rec.price = std::stod(msg.get_value_of_key(sajson::string("price",5)).as_string());     
        rec.side = exchange::coinbase::convert_side_type(msg.get_value_of_key(sajson::string("side", 4)).as_string());
    }

    // {"type": "received","time": "2014-11-09T08:19:27.028459Z","product_id": "BTC-USD","sequence": 12,"order_id": "dddec984-77a8-460a-b958-66f114b0de9b","funds": "3000.234","side": "buy","order_type": "market"}
    void parse_json_full_received_market(const sajson::value &msg, exchange::coinbase::ws_full_channel_received_market &rec)
    {
        rec.receive_time = std::chrono::system_clock::now();
        rec.product_id = exchange::coinbase::convert_product_id(msg.get_value_of_key(sajson::string("product_id", 10)).as_string());    
        rec.time = parse_date(msg.get_value_of_key(sajson::string("time", 4)).as_string());
        msg.get_value_of_key(sajson::string("sequence",8)).get_int53_value(&rec.sequence);
        strcpy(rec.order_id, msg.get_value_of_key(sajson::string("order_id",8)).as_cstring());
        rec.side = exchange::coinbase::convert_side_type(msg.get_value_of_key(sajson::string("side", 4)).as_string());
        // TODO maybe set funds to -1 if not in packet since it is optional 
        const sajson::string funds("funds",5);
        const sajson::value res = msg.get_value_of_key(funds);
        if (res.get_type() != sajson::type::TYPE_NULL)
            rec.funds = std::stod(msg.get_value_of_key(funds).as_string());     
    }

    //{"type": "open","time": "2014-11-07T08:19:27.028459Z","product_id": "BTC-USD","sequence": 10,"order_id": "d50ec984-77a8-460a-b958-66f114b0de9b","price": "200.2","remaining_size": "1.00","side": "sell"}
    void parse_json_full_open(const sajson::value &msg, exchange::coinbase::ws_full_channel_open &open)
    {
        open.receive_time = std::chrono::system_clock::now();
        open.time =  parse_date(msg.get_value_of_key(sajson::string("time", 4)).as_string());
        open.product_id = exchange::coinbase::convert_product_id(msg.get_value_of_key(sajson::string("product_id", 10)).as_string());
        msg.get_value_of_key(sajson::string("sequence",8)).get_int53_value(&open.sequence);
        strcpy(open.order_id, msg.get_value_of_key(sajson::string("order_id",8)).as_cstring());
        open.price = std::stod(msg.get_value_of_key(sajson::string("price",5)).as_string());
        open.remaining_size = std::stod(msg.get_value_of_key(sajson::string("remaining_size",14)).as_string());
        open.side = exchange::coinbase::convert_side_type(msg.get_value_of_key(sajson::string("side", 4)).as_string());
    }

    // {"type": "done","time": "2014-11-07T08:19:27.028459Z","product_id": "BTC-USD","sequence": 10,"price": "200.2","order_id": "d50ec984-77a8-460a-b958-66f114b0de9b","reason": "filled", (|| or "canceled")"side": "sell","remaining_size": "0"}
    // MArket orders wil not have a remaining size or price !!!! 
    void parse_json_full_done(const sajson::value &msg, exchange::coinbase::ws_full_channel_done &done)
    {
        done.receive_time = std::chrono::system_clock::now();
        done.time = parse_date(msg.get_value_of_key(sajson::string("time", 4)).as_string());
        done.product_id = exchange::coinbase::convert_product_id(msg.get_value_of_key(sajson::string("product_id", 10)).as_string());
        msg.get_value_of_key(sajson::string("sequence",8)).get_int53_value(&done.sequence);
        strcpy(done.order_id, msg.get_value_of_key(sajson::string("order_id",8)).as_cstring());
        done.reason = exchange::coinbase::convert_reason_type(msg.get_value_of_key(sajson::string("reason", 6)).as_string());
        done.side = exchange::coinbase::convert_side_type(msg.get_value_of_key(sajson::string("side", 4)).as_string());
        // market orders won't have these fields!!!  // according to docs but it looks like these is always a reamining size of 0 if market order :( 
        const sajson::string remaining_size("remaining_size",14);
        const sajson::string price("price",5);
        const sajson::value res1 = msg.get_value_of_key(remaining_size);
        const sajson::value res2 = msg.get_value_of_key(price);
        if (res1.get_type() != sajson::type::TYPE_NULL)
            done.remaining_size = std::stod(msg.get_value_of_key(sajson::string("remaining_size",14)).as_string());
        if (res2.get_type() != sajson::type::TYPE_NULL)
            done.price = std::stod(msg.get_value_of_key(sajson::string("price",5)).as_string());
    }

    //{"type": "match","trade_id": 10,"sequence": 50,"maker_order_id": "ac928c66-ca53-498f-9c13-a110027a60e8","taker_order_id": "132fb6ae-456b-4654-b4e0-d681ac05cea1","time": "2014-11-07T08:19:27.028459Z","product_id": "BTC-USD","size": "5.23512","price": "400.23","side": "sell"}
    void parse_json_full_match(const sajson::value &msg, exchange::coinbase::ws_full_channel_match &match)
    {
        match.receive_time = std::chrono::system_clock::now();
        msg.get_value_of_key(sajson::string("trade_id",8)).get_int53_value(&match.trade_id);
        msg.get_value_of_key(sajson::string("sequence",8)).get_int53_value(&match.sequence);
        strcpy(match.maker_order_id, msg.get_value_of_key(sajson::string("maker_order_id",14)).as_cstring());
        strcpy(match.taker_order_id, msg.get_value_of_key(sajson::string("taker_order_id",14)).as_cstring());
        match.time = parse_date(msg.get_value_of_key(sajson::string("time", 4)).as_string());
        match.product_id = exchange::coinbase::convert_product_id(msg.get_value_of_key(sajson::string("product_id", 10)).as_string());
        match.size = std::stod(msg.get_value_of_key(sajson::string("size",4)).as_string());
        match.price = std::stod(msg.get_value_of_key(sajson::string("price",5)).as_string());
        match.side = exchange::coinbase::convert_side_type(msg.get_value_of_key(sajson::string("side", 4)).as_string());
    }

    // {"type": "change","time": "2014-11-07T08:19:27.028459Z","sequence": 80,"order_id": "ac928c66-ca53-498f-9c13-a110027a60e8","product_id": "BTC-USD","new_size": "5.23512","old_size": "12.234412","price": "400.23","side": "sell"}
    void parse_json_full_change(const sajson::value &msg, exchange::coinbase::ws_full_channel_change &change)
    {
        change.receive_time = std::chrono::system_clock::now();
        change.time = parse_date(msg.get_value_of_key(sajson::string("time", 4)).as_string());
        msg.get_value_of_key(sajson::string("sequence",8)).get_int53_value(&change.sequence); 
        strcpy(change.order_id, msg.get_value_of_key(sajson::string("order_id",8)).as_cstring());
        change.product_id = exchange::coinbase::convert_product_id(msg.get_value_of_key(sajson::string("product_id", 10)).as_string());
        change.price = std::stod(msg.get_value_of_key(sajson::string("price",5)).as_string());
        change.side = exchange::coinbase::convert_side_type(msg.get_value_of_key(sajson::string("side", 4)).as_string());
        // change can either have 
        //      new size and old_Size 
        //  or
        //      new_funds and old_funds
        //      TODO this is a bit smeely here ... 
        change.new_size = std::stod(msg.get_value_of_key(sajson::string("new_size",8)).as_string());
        change.old_size = std::stod(msg.get_value_of_key(sajson::string("old_size",8)).as_string());
        change.new_funds = std::stod(msg.get_value_of_key(sajson::string("new_funds",9)).as_string());
        change.old_funds = std::stod(msg.get_value_of_key(sajson::string("old_funds",9)).as_string());
    }

    // {"type": "activate","product_id": "test-product","timestamp": "1483736448.299000","user_id": "12","profile_id": "30000727-d308-cf50-7b1c-c06deb1934fc","order_id": "7b52009b-64fd-0a2a-49e6-d8a939753077","stop_type": "entry","side": "buy","stop_price": "80","size": "2","funds": "50","taker_fee_rate": "0.0025","private": true}
    void parse_json_full_activate(const sajson::value &msg, exchange::coinbase::ws_full_channel_margin_activate &activate)
    {
        activate.product_id = exchange::coinbase::convert_product_id(msg.get_value_of_key(sajson::string("product_id", 10)).as_string());        
        activate.timestamp = std::stod(msg.get_value_of_key(sajson::string("timestamp",9)).as_string());
        strcpy(activate.user_id, msg.get_value_of_key(sajson::string("user_id",7)).as_cstring());
        strcpy(activate.profile_id, msg.get_value_of_key(sajson::string("profile_id",10)).as_cstring());
        strcpy(activate.order_id, msg.get_value_of_key(sajson::string("order_id",8)).as_cstring());
        activate.stop_of_type = exchange::coinbase::stop_type::entry;
        activate.side = exchange::coinbase::convert_side_type(msg.get_value_of_key(sajson::string("side", 4)).as_string());
        activate.stop_price = std::stod(msg.get_value_of_key(sajson::string("stop_price",10)).as_string());
        activate.size = std::stod(msg.get_value_of_key(sajson::string("size",4)).as_string());
        activate.funds = std::stod(msg.get_value_of_key(sajson::string("funds",5)).as_string());
        activate.taker_fee_rate = std::stod(msg.get_value_of_key(sajson::string("taker_fee_rate",14)).as_string());
        // this last one can either be  true or a false 
        if (msg.get_value_of_key(sajson::string("private",7)).get_type() == sajson::TYPE_TRUE)
            activate.margin_private = true;
        else
            activate.margin_private = false;
    }

    void parse_json(const sajson::document& doc)
    {
        const sajson::value node = is_valid(doc);

        // first find out what type of message this is
        const sajson::string type_key("type",4);
        std::string type_val = node.get_value_of_key(type_key).as_string(); // we know it will be a string! 

        // now lets parse packets depending on the type of 
        if (type_val == "error")
        { 
            parse_json_error(node);
        } 
        else if(type_val == "subscriptions")
        {
            parse_json_subscriptions(node);
        }
        else if(type_val == "heartbeat")
        {
            exchange::coinbase::ws_heartbeat hb;
            parse_json_heartbeat(node, hb);
        }
        else if(type_val == "ticker")
        {
            exchange::coinbase::ws_ticker tick;
            parse_json_ticker(node, tick);
        }
        else if(type_val == "l2update")
        {
            exchange::coinbase::ws_l2_update update;
            parse_json_l2update(node, update);
        }
        else if(type_val == "snapshot")
        {
            exchange::coinbase::ws_l2_snapshot snapshot;
            parse_json_l2snapshot(node, snapshot);
        }
        else if(type_val == "open")
        {
            exchange::coinbase::ws_full_channel_open open;
            parse_json_full_open(node, open);
        }
        else if(type_val == "received")
        {
            const sajson::string limit_type("order_type",10);
            std::string limit_val = node.get_value_of_key(limit_type).as_string();
            if (limit_val == "limit")
            {
                exchange::coinbase::ws_full_channel_received_limit rec;
                parse_json_full_received(node, rec);
            }
            else // market order
            {
                exchange::coinbase::ws_full_channel_received_market rec;
                parse_json_full_received_market(node, rec);
            }
        }
        else if(type_val == "done")
        {
            exchange::coinbase::ws_full_channel_done done;
            parse_json_full_done(node, done);
        }
        else if(type_val == "match")
        {
            exchange::coinbase::ws_full_channel_match match;;
            parse_json_full_match(node, match);
        }
        else if(type_val == "change")
        {
            exchange::coinbase::ws_full_channel_change change;
            parse_json_full_change(node, change);
        }
        else if(type_val == "activate")
        {
            exchange::coinbase::ws_full_channel_margin_activate activate;
            parse_json_full_activate(node, activate);
        }

    }

}}} // exchange::coinbase::websocket
 
#endif 

