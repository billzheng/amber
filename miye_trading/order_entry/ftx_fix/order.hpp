#pragma once
#include "libcore/types/types.hpp"

#include <cstdint>
#include "../ftx_ftx/fix_enum.hpp"

namespace miye::trading::fix
{

using order_id_t   = uint64_t;
using clorder_id_t = uint64_t;
using price_t      = double;
using quantity_t   = uint32_t;

// using namespace types;

enum class order_status_t : uint8_t
{
    undefined       = 0,
    new_order       = 1,
    new_replacement = 2,
    booked          = 3,
    pending_replace = 4,
    pending_close   = 5,
    closed          = 6
};

struct order_t
{
    order_t() = default;
    //    order_t(order_id_t order_id, clorder_id_t clord_id, clorder_id_t clord_id2, types::instrument_id_t
    //    qcl_instrument, quantity_t qty, price_t price, SideEnum side, order_status_t status) : order_id(order_id) ,
    //    clord_id(clord_id) , clord_id2(clord_id2) , qcl_instrument(qcl_instrument) , qty(qty) , price(price) ,
    //    side(side) , status(status)
    //    {
    //    }

    order_t(const types::message::order_insert& oi, uint32_t seq)
        : order_id(oi.order_key), clord_id(oi.cl_order_key.key), md_key(oi.md_key), side(oi.side), qty(oi.volume),
          instrument(oi.instrument), price(oi.price), duration(oi.duration), fix_sequence(seq)
    {
    }

    types::order_key_t order_id;
    types::order_key_t clord_id;
    types::order_md_key md_key{0};
    types::order_key_t exchange_key{0};
    types::message::side_t side{};
    quantity_t qty{};
    types::instrument_id_t instrument{};
    price_t price{};
    types::message::duration_t duration{};
    order_status_t status{order_status_t::undefined};
    types::message::msg_type_t last_request{types::message::msg_type_t::order_insert};
    uint32_t fix_sequence{};
};

inline std::ostream& operator<<(std::ostream& os, const order_t& o)
{
    const char delimiter = ',';
    os << o.order_id << delimiter << o.clord_id << delimiter << o.instrument << delimiter << o.qty << delimiter
       << o.price << delimiter << o.duration << delimiter << o.side << delimiter << +toUnderlyingType(o.status)
       << delimiter << o.last_request << delimiter << o.md_key.key << delimiter << o.fix_sequence;

    return os;
}

} // namespace miye::trading::fix
