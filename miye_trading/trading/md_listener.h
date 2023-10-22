#pragma once

#include "libcore/types/types.hpp"

#include <stdint.h>

namespace miye::trading
{

struct MDListener
{
    virtual ~MDListener() = default;

    virtual int32_t onSnapshotFinished(int32_t cid) = 0;
    virtual int32_t onBookChange(int32_t cid)       = 0;
    virtual int32_t onTrade(uint64_t timestamp, int32_t cid, uint64_t tradeId, Side side, price_t price, quantity_t qty,
                            bool isDone)            = 0;
    virtual int32_t onTick(int32_t cid)             = 0;
};

} // namespace miye::trading
