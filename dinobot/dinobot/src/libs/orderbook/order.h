#ifndef _DINOBOT_ORDERBOOK_ORDER_H
#define _DINOBOT_ORDERBOOK_ORDER_H

#include <array>
#include <memory>

namespace dinobot { namespace orderbook {

using price_t = uint64_t;
using quantity_t = uint64_t;

// maybe have a message type insterted here ie , quote, trade, cancel ammend etc ??
//template <typename OID>
struct order_t 
{
    enum class side : char 
    { 
        buy = 'b', 
        sell = 's'
    };

    //using id_t = OID;
    using side_t = side;
    using id_t = uint64_t;
    using sptr = std::shared_ptr<order_t>;

    order_t(id_t id, side_t type, price_t price, quantity_t quantity)
        : id(id), side(type), price(price), quantity(quantity) {}

    id_t        id;
    side_t      side;
    price_t     price; 
    quantity_t  quantity; 

    bool        is_buy() const { return side == side::buy; }

    // output printing
    friend std::ostream& operator<< (std::ostream &o, const order_t &a)
    {
        char res = 's';
        if (a.side == side_t::buy)
            res = 'b';

        o << " [ " << a.id << " " << res << /* " " << a.price <<*/ "  " << a.quantity << " ] ";

        return o;
    }
};


struct level_details_t
{
    price_t price;
    quantity_t quantity;
    uint32_t count;
};

}} // dinobot :: orderbook
#endif
