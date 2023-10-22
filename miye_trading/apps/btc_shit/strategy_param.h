#pragma once

#include "trading/instrument.h"
#include <vector>

namespace miye::trading::ftx
{

struct StrategyParam
{
    std::vector<instrument_t> instruments;
};

} // namespace miye::trading::ftx
