#pragma once
#include <stdint.h>

namespace miye
{
namespace trading
{
const constexpr int32_t INVALID_CID{-1};

enum class Exchange : uint16_t
{
    BINANCE = 0,
    FTX
};

inline const char* toString(Exchange exchange)
{
    switch (exchange)
    {
    case Exchange::BINANCE:
        return "BINANCE";
    case Exchange::FTX:
        return "FTX";
    default:
        return "UNKNOWN";
    }
    return "UNKNOWN";
}

} // namespace trading
} // namespace miye
