#ifndef _DINOBOT_ORDERBOOK_ORDERBOOK_H
#define _DINOBOT_ORDERBOOK_ORDERBOOK_H

#include <iostream>
#include <functional>
#include <map>
#include <string>
#include <unordered_map>
#include <iterator>

#include "price_level.h"
#include "order.h"

namespace dinobot { namespace orderbook {

/*
 * Order book for a particular symbol
 */
class orderbook {
public:
    orderbook() : sequence_(0), total_orders_on_ask_(0), total_orders_on_bid_(0) {}
    ~orderbook() {}

    void add(const order_t::id_t&, price_t, quantity_t, order_t::side_t);
    void cancel(const order_t::id_t&);
    void amend(const order_t::id_t &);
    
    void execute(const order_t::id_t &, quantity_t);

    // general operations we may ask of our order book 
    size_t recalc_num_ask_orders()
    {
        total_orders_on_ask_ = 0;
        for (const auto& kv : asks_)
            total_orders_on_ask_ += kv.second.size();
        return total_orders_on_ask_;
    }
    size_t recalc_num_bid_orders() 
    {
        total_orders_on_bid_ = 0;
        for (const auto& kv : bids_)
            total_orders_on_bid_ += kv.second.size();
        return total_orders_on_bid_;
    }

    size_t num_ask_orders() const { return total_orders_on_ask_;}
    size_t num_bid_orders() const { return total_orders_on_bid_;}
    size_t num_orders() const { return total_orders_on_bid_ + total_orders_on_ask_; }

    size_t num_ask_price_levels() const { return asks_.size(); }
    size_t num_bid_price_levels() const { return bids_.size(); }
    size_t num_price_levels() const { return num_ask_price_levels() + num_bid_price_levels(); }

    // Inner market
    bool changed() const { return bbo_changed_; }

    // TODO stuff with volumes/counts per price level 
    // maybe have funcitons that take a uint ie 0 for level 1 1 for lovel2 etc 
    // then return what we want from the price level class
    /*
    price_t get_price_level_detail(uint8_t level, order_t::side_t s) 
    {
        price_t px = 0;
        std::map<uint32_t, price_level>::iterator it;

        if (s == order_t::side_t::buy)
            it = bids_.rbegin();
        else 
            it = asks_.begin();

        if (it.size() >= level)
        {
            //auto x  = it.begin();
            auto z  = std::next(it, level);
            px = (z->second)->price;
        }
        else
            std::cout << "what to do here, throw?" << std::endl;
        return px;
    }
    */

    void print_book();
private:
    uint64_t sequence_;
    size_t total_orders_on_ask_;
    size_t total_orders_on_bid_;

    // price , qty, count per level is needed here // so need tools to do the job here ... 
    // return a level_details struct 

    std::map<uint32_t/*price_t type?? */, price_level> asks_, bids_;

    // Cache for fast order access using ID.
    // We use weak pointers since the cache doesn't 'own' m_orders
    std::unordered_map<order_t::id_t, std::weak_ptr<order_t>> orders_cache_;

    // Current best ask and bid
    price_t best_ask_ = 0;
    price_t best_bid_ = 0;
    void update_bbo()
    { //TODO check if not empty ?
	    auto a = asks_.begin();
	    auto b = bids_.rbegin();
	    if (a->first != best_ask_ || b->first != best_bid_)
	    {
	        bbo_changed_ = true;
    	    best_ask_ = a->first;
	        best_bid_ = b->first;
	    }
    }

    // Flag indicating best ask/bid change
    bool bbo_changed_ = false;
};

}} // dinobot :: orderbook
#endif
