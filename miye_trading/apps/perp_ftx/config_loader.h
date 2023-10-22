#pragma once
#include "libcore/types/types.hpp"
//#include "libcore/utils/number_utils.hpp"
#include "libs/inifile/inicpp.h"
#include "trading/instrument.h"

#include <string>
#include <vector>

namespace miye::trading::ftx
{

struct ConfigLoader
{
    static std::string getLogFile(std::string date, std::string filename)
    {
        auto iniFile        = ini::IniFile(filename);
        auto const log_path = iniFile["log"]["log_path"].as<std::string>();
        auto const log_file = iniFile["log"]["log_file"].as<std::string>();
        return log_path + '/' + date + '/' + log_file;
    }

    static std::vector<instrument_t> getInstrumentInfo(std::string filename)
    {
        std::vector<instrument_t> instruments;
        auto iniFile = ini::IniFile(filename);
        instrument_t instrument{};

        instrument.symbol           = iniFile["markets.ftx.instrument.ftm-perp"]["symbol"].as<std::string>();
        instrument.tickSize         = iniFile["markets.ftx.instrument.ftm-perp"]["tick_size"].as<double>();
        instrument.qtyStep          = iniFile["markets.ftx.instrument.ftm-perp"]["qty_step"].as<double>();
        instrument.imfFactor        = iniFile["markets.ftx.instrument.ftm-perp"]["imf_factor"].as<double>();
        auto const positionLimitWgt = iniFile["markets.ftx.instrument.ftm-perp"]["position_limit_wgt"];
        if (positionLimitWgt.as<std::string>() != "na")
        {
            instrument.positionLimitWgt = positionLimitWgt.as<double>();
        }

        instruments.push_back(instrument);
        return instruments;
    }
};

} // namespace miye::trading::ftx
