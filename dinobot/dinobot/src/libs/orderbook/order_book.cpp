#include <cassert>
#include "order_book.h"

namespace dinobot { namespace orderbook {

/*orderbook::~orderbook()
{
}*/

/*
 * Add order of the given type, price and quantity
 */
void orderbook::add(const order_t::id_t& id, price_t price, quantity_t quantity, order_t::side_t type)
{
    // Check for duplicate add
    auto it = orders_cache_.find(id);
    assert(it == orders_cache_.end());

    // Create a new order
    auto  ord         = std::make_shared<order_t>(id, type, price, quantity);
    auto& side        = ord->is_buy() ? bids_ : asks_;
    auto& price_level = side[price];

    // Add the order to the cache
    orders_cache_[id] = ord;

    // Put the new order to the end of the price level
    price_level.push_back(ord);

    if (ord->is_buy())
	total_orders_on_bid_++;
    else
	total_orders_on_ask_++;
    update_bbo();
}

/*
 * delete / cancel from order book 
 */
void orderbook::cancel(const order_t::id_t& id)
{
    auto it = orders_cache_.find(id);
    if (it == orders_cache_.end())
        return;

    auto  ord         = it->second.lock();
    auto& side        = ord->is_buy() ? bids_ : asks_;
    auto& price_level = side[ord->price];

    // Delete our order from the price level
    price_level.remove(id);

    // If it was the last order, drop the price level
    if (price_level.empty())
        side.erase(ord->price);

    // Finally, delete the order from the cache
    orders_cache_.erase(it);

    if (ord->is_buy() )
	    total_orders_on_bid_--;
    else
	    total_orders_on_ask_--;
    update_bbo();
}

/*
 * Execute given quantity of vol of the order.
 */
void orderbook::execute(const order_t::id_t& id, quantity_t quantity)
{
    auto it = orders_cache_.find(id);
    if (it == orders_cache_.end())
        return;

    assert(!it->second.expired());
    auto ord = it->second.lock();
    quantity   = std::min(quantity, ord->quantity); // TODO - what this needed for 

    // For buy orders, we need to match with asks and vice versa
    auto& side = ord->is_buy() ? asks_ : bids_;

    // We start from the lowest sell price
    auto lowest = asks_.begin();

    // We start from highest buy price
    auto highest = bids_.end();

    if (!bids_.empty())
        --highest;
    /*XXX*/std::cout << "high: " << highest->first << " lowest  "  << lowest->first << std::endl;
    while (quantity > 0 && ord->quantity > 0 && !side.empty()) {
        auto& price_level = ord->is_buy() ? lowest->second : highest->second;
        auto  oldest      = price_level.front().get();
        /*XXX*/std::cout << "XXXXXX oldest: " << *oldest << " ord: " << *ord <<  " qty: " << quantity << std::endl;
        // We have enough vol on the oldest order
        if (ord->quantity >= quantity) {
            ord->quantity -= quantity;

    	    // TODO maybe update the price level volume ... 
	        price_level.modify_volume(quantity);

            break;
        }
        else {
            ord->quantity -= oldest->quantity;
            quantity -= oldest->quantity;

            orders_cache_.erase(oldest->id);
            price_level.pop_front();
            // If the price level is empty, delete it and go to the next one
            if (price_level.empty()) {
                if (ord->is_buy())
                    asks_.erase(lowest++);
                else
                    bids_.erase(highest--);
            }
        }
    }

    // Remove our order if it was fully filled
    if (ord->quantity == 0)
        cancel(id);

    update_bbo();
}

// ask  price   [oid, volume] [oid, volume] [oid, volume]
// ask  price++ [oid, volume] [oid, volume] [oid, volume]
//
// bid  price   [oid, volume] [oid, volume] [oid, volume]
// bid  price-- [oid, volume] [oid, volume] [oid, volume]
void orderbook::print_book()
{
    std::map<uint32_t, price_level>::reverse_iterator buy_it;
    std::map<uint32_t, price_level>::reverse_iterator ask_it;

    std::cout << "printing the orderbook" << std::endl;
    for (ask_it = asks_.rbegin(); ask_it != asks_.rend(); ask_it++)
    {
        std::cout<< "asks:\t" << ask_it->first << " ";
        (ask_it->second).print_level();
    }

    std::cout << " " << std::endl;

    for (buy_it = bids_.rbegin(); buy_it != bids_.rend(); buy_it++)
    {
        std::cout << "bids:\t" << buy_it->first << " " ;
        (buy_it->second).print_level();
    }

}


}} // dinobot:: orderbook
