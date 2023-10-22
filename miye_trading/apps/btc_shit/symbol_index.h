#pragma once
#include <stdint.h>

namespace miye::trading::ftx
{

enum class SymbolIndex : int16_t
{
    Invalid         = -1,
    BINANCE_BTCUSDT = 0, //
    FTX_SHIT_PERP   = 1,
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
    case SymbolIndex::BINANCE_BTCUSDT:
        return "BINANCE:BTCUSDT";
    case SymbolIndex::FTX_SHIT_PERP:
        return "FTX:SHIT-PERP";
    }
    return "UNKNOWN";
}

} // namespace miye::trading::ftx
