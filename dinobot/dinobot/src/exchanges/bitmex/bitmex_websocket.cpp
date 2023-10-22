#include <thread>
#include <cstring>


#include "bitmex_websocket.h"
#include "bitmex_messages.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "../../libs/json/fast/sajson.h"
#pragma GCC diagnostic pop

namespace dinobot { namespace exchanges { namespace bitmex {


bitmex_websocket::~bitmex_websocket() 
{
    // TODO delete the objects and close ... 
    //stop threads then unsubscribe ..
}


void bitmex_websocket::unsubscribe()
{
    
}

void bitmex_websocket::start()
{
    running_ = false;
    started_ = true;

    thread_.push_back(std::thread(&bitmex_websocket::init, this));
}

void bitmex_websocket::parse_json(char * msg, size_t len, uint64_t ts)
{
    auto xxx = std::string(msg, len) ; /// XXX 
	//std::cout << "XXXX " << xxx << std::endl;
	//std::cout << "~~" << std::endl;

    const sajson::document& doc = sajson::parse(sajson::dynamic_allocation(), sajson::mutable_string_view(len, msg));

    if (!doc.is_valid())
    {
        std::cerr << "XX: " << doc.get_error_message_as_cstring() << std::endl;
        throw std::runtime_error("We have received an invalid JSON payload, exiting");
    }

    // all json payloads we receive should be an OBJECT type, so lets test for it
    const sajson::value node = doc.get_root();
    if (node.get_type() != sajson::TYPE_OBJECT)
    {
        throw std::runtime_error("We have received an invalid JSON payload, exiting");
    }
    //std::cout << "################## after doc parse ###############################" << std::endl;

    std::string type_val;
    auto type_val1 = node.get_value_of_key(exchange::bitmex::cache::cache::table);
    if (type_val1.get_type() == sajson::TYPE_NULL)
        return; // fix this up  // TODO be able to parse // parital, welcome, subscription_success messages
    else
        type_val = type_val1.as_string();

    //std::cout << "!!!! : starting !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
    if (type_val == "orderBookL2")
    {
        std::string action_val = node.get_value_of_key(exchange::bitmex::cache::action).as_string();
        if (action_val == "insert")
        {
            exchange::bitmex::parse_orderbook_insert(node, ts);
        }
        else if (action_val == "update")
        {
            exchange::bitmex::parse_orderbook_update(node, ts);
        }
        else if (action_val == "delete")
        {
            exchange::bitmex::parse_orderbook_delete(node, ts);
        }
        else // partial 
        {
            exchange::bitmex::parse_orderbook_partial(node);
        }
    }
    else if (type_val == "trade")
    {
	    exchange::bitmex::parse_trade_insert(node, ts);
    }
    else if (type_val == "quote")
    {
        exchange::bitmex::parse_quote_insert(node, ts);
    }
    // all the other messages we care about after here 
    // TODO skip for the time being ... 

}

}}} // dinobot::exchanges::bitmex

