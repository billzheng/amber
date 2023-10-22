#ifndef _DINOBOT_ORDERBOOK_PRICELEVEL_H
#define _DINOBOT_ORDERBOOK_PRICELEVEL_H 
/*
 * simple order book to just get us going...
 */
#include <algorithm>
#include <list>

#include "order.h"

namespace dinobot { namespace orderbook {

/*
 * Price level is a list of orders maintained as FIFO queue
 */
class price_level {
public:
    price_level() {}
    ~price_level() {}

    size_t   size() const { return level_queue_.size(); }
    bool     empty() const { return level_queue_.empty(); }

    uint64_t total_volume() const { return total_volume_; }
    void modify_volume(uint64_t v) { total_volume_ -= v; } // on partial fill vol will decreade

    const order_t::sptr& front() const { return level_queue_.front(); }

    // FIFO operations 
    void pop_front() // used by 'trade' packets
    { 
        auto o = front();
        total_volume_ -= o->quantity;

        level_queue_.pop_front(); 
    }

    void push_back(const order_t::sptr &order)  // used by 'add' packets
    { 
        total_volume_ += order->quantity;
        level_queue_.push_back(order); 
    }

    void remove(const order_t::id_t id)
    {
        level_queue_.remove_if([&id](const auto& o) { return o->id == id; });
    }

    void print_level()
    {
        for (auto &o: level_queue_)
            std::cout << " " << *o ;
        std::cout << std::endl;
    }

private:
    std::list<order_t::sptr> level_queue_;
    uint64_t total_volume_;
};

}} // dinobot::orderbook

#endif
