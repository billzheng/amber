#pragma once
#include <stdint.h>

namespace dinobot { namespace exchanges {
//using version_t = uint8_t;


enum class exchange_id_t : uint8_t
{
    undefined       = 0,
    bitfinex        = 1, 
    bitmex          = 2,
    bitstamp        = 3,
    coinbasepro     = 4,
    kraken          = 5,
    okcoin          = 6,
    poloniex        = 7,
    derbit          = 8, 
};

enum class product_id_t : uint8_t
{
    undefined       = 0,
    currency_pair   = 1,
    future          = 2,
    option          = 3,
    perpetual_swap  = 4,
};

enum class currencies_t : uint8_t
{
    undefined       = 0,
    BTC             = 1,
    EOS             = 2,
    ETH             = 3,
    EUR             = 4,
    GBP             = 5,
    JPY             = 6,
    USD             = 7
};

enum class trading_pairs_t : uint32_t
{
    undefined       = 0,
    btcusd          = 1,
    btcusddd         = 1
};

} // exchanges
} // dinobot
