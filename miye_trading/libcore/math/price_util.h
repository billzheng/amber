#pragma once
#include "libcore/types/types.hpp"
#include "math_util.h"

#include <cassert>
#include <cmath>
#include <stdint.h>

namespace miye
{
namespace trading
{
namespace math
{

inline double roundDown(double price, double tickSize) noexcept
{
    return std::floor(price / tickSize + EPOSILON) * tickSize;
}

inline double roundToBid(double price, double tickSize) noexcept
{
    return roundDown(price, tickSize);
}
inline double roundUp(double price, double tickSize) noexcept
{
    return std::ceil(price / tickSize - EPOSILON) * tickSize;
}

inline double roundToAsk(double price, double tickSize) noexcept
{
    return roundUp(price, tickSize);
}

inline double roundToTick(Side side, double price, double tickSize) noexcept
{
    assert(tickSize > 0);

    if (side == Side::BUY)
    {
        return roundToBid(price, tickSize);
    }
    if (side == Side::SELL || side == Side::SHORT_SELL)
    {
        return roundToAsk(price, tickSize);
    }
    return price;
}

inline double roundLot(double qty, double lotSize)
{
    assert(lotSize > 0);
    return std::floor(qty / lotSize) * lotSize;
}

} // namespace math
} // namespace trading
} // namespace miye
