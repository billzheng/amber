#pragma once
#include <stdint.h>

namespace miye
{
namespace trading
{
namespace binance
{

enum class EventType : uint16_t
{
    UNKNOWN = 0,
    DepthUpdate = 1,
    Trade,
    AggTrade,
    MarkPrice,
    BookTicker
};

inline EventType fromEventTypeStr(const std::string& type)
{
    if (type == "depthUpdate")
    {
        return EventType::DepthUpdate;
    }
    else if (type == "trade")
    {
        return EventType::Trade;
    }
    else if (type == "aggTrade")
    {
        return EventType::AggTrade;
    }
    else if (type == "bookTicker")
    {
        return EventType::BookTicker;
    }
    return EventType::UNKNOWN;
}

} // namespace binance
} // namespace trading
} // namespace miye
