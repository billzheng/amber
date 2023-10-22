#pragma once
#include <stdint.h>
#include <string>

namespace miye
{
namespace trading
{
namespace ftx
{

enum class Channel : uint16_t
{
    UNKNOWN,
    ORDERBOOK, // orderbook market data
    TRADES,    // trade market data
    TICKER     // bbo market data
};

inline const char* toString(Channel channel)
{
    return "UNKNOWN";
}

inline Channel fromChannelStr(const std::string& o)
{
    if (o == "trades")
    {
        return Channel::TRADES;
    }
    else if (o == "orderbook")
    {
        return Channel::ORDERBOOK;
    }
    else if (o == "ticker")
    {
        return Channel::TICKER;
    }
    return Channel::UNKNOWN;
}

// https://docs.ftx.com/#request-process response format
enum class MsgType : uint16_t
{
    UNKNOWN,
    ERROR,
    SUBSCRIBED,
    UNSUBSCRIBED,
    INFO,
    PARTIAL, // contains snapshot of current market data
    UPDATE
};

inline MsgType fromMsgTypeString(const std::string& msgType)
{
    // most of the book data is "update" msgs
    if (msgType == "update")
    {
        return MsgType::UPDATE;
    }
    else if (msgType == "partial")
    {
        return MsgType::PARTIAL;
    }
    else if (msgType == "error")
    {
        return MsgType::ERROR;
    }
    else if (msgType == "subscribed")
    {
        return MsgType::SUBSCRIBED;
    }
    else if (msgType == "unsubscribed")
    {
        return MsgType::UNSUBSCRIBED;
    }
    else if (msgType == "info")
    {
        return MsgType::INFO;
    }

    return MsgType::UNKNOWN;
}

inline const char* toString(MsgType type)
{
    switch (type)
    {
    case MsgType::UPDATE:
        return "update";
    case MsgType::ERROR:
        return "error";
    case MsgType::PARTIAL:
        return "partial";
    case MsgType::INFO:
        return "info";
    case MsgType::SUBSCRIBED:
        return "subscribed";
    case MsgType::UNSUBSCRIBED:
        return "unsubscribed";
    case MsgType::UNKNOWN:
        return "unknown";
    }
    return "unknown";
}

} // namespace ftx
} // namespace trading
} // namespace miye
