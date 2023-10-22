#pragma once
//#include "hedger.h"
#include "libs/ftx/rest/client.h"
#include "libs/logger/logger.hpp"
//#include "maker.h"
#include "position/position.h"
#include "risk/risk.h"
#include "strategy_param.h"
//#include "taker.h"
#include <stdint.h>

namespace miye::trading::ftx
{
struct Context
{
    logger::Logger* logger_{};
    // position
    std::vector<symbol_t> symbols;
    position::PortforlioPositions positions;
    // risk
    risk::Risk risk{positions, symbols};
    // OrderManager om{};

    // strategy
    // Taker taker{risk, positions, symbols, om};
    // Hedger hedger{risk, positions, symbols, om};
    // hedger
    StrategyParam stratParam{};
    // OM

    ::ftx::RESTClient client;

    void setSymbols(std::vector<symbol_t> value) { this->symbols = value; }

    logger::Logger* logger() { return logger_; }
    int32_t init(logger::Logger* logger, std::string config);
};
} // namespace miye::trading::ftx
