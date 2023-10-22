#pragma once
#include <stdint.h>

namespace miye
{
namespace trading
{

enum class SymbolIndex : int16_t
{
    Invalid         = -1,
    BINANCE_FTMUSDT = 0, // binance sol-perp
    FTX_FTM_PERP    = 1,
    NUM_SYMBOLS     = 2
};

/*
 * convert the SymbolIndex string to be the same as global symbol e.g.
 * exchange:symbol
 */
inline const char* toString(SymbolIndex idx)
{
    switch (idx)
    {
    case SymbolIndex::BINANCE_FTMUSDT:
        return "BINANCE:FTMUSDT";
    case SymbolIndex::FTX_FTM_PERP:
        return "FTX:FTM-PERP";
    default:
        return "UNKNOWN";
    }
    return "UNKNOWN";
}

} // namespace trading
} // namespace miye
