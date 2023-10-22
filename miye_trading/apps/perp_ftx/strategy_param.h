#pragma once

#include "trading/instrument.h"
#include <vector>

namespace miye
{
namespace trading
{
namespace ftx
{

struct StrategyParam
{
    std::vector<instrument_t> instruments;
};

} // namespace ftx
} // namespace trading
} // namespace miye
