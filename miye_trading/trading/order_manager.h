#pragma once

#include "libcore/types/types.hpp"

namespace miye::trading::ftx
{

struct CancelInfo
{
    int32_t totalQty{};
    int32_t orderNum{};
};

struct OrderManager
{
    int32_t placeOrder(const symbol_t& symbol, Side side, price_t price, quantity_t qty) { return 0; }

    // onConfirm, onCancel, onReject, onCancelReject

    CancelInfo cancelAll(const account_t& account, Side side)
    {
        CancelInfo ci{};
        return ci;
    }
    CancelInfo cancelAll(const account_t& account, const symbol_t& symbol)
    {
        CancelInfo ci{};
        return ci;
    }
    CancelInfo cancelAll(const symbol_t& symbol)
    {
        CancelInfo ci{};
        return ci;
    }
    CancelInfo cancelAll()
    {
        CancelInfo ci{};
        return ci;
    }
    CancelInfo cancelFrom(const symbol_t& symbol, Side side, price_t price)
    {
        CancelInfo ci{};
        return ci;
    }
};

} // namespace miye::trading::ftx
