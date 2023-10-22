#pragma once
#include "../../git_version.h"
#include "config_loader.h"
#include "cxxopts.hpp"
#include "libcore/essential/app.hpp"
#include "market_data/market_main.h"
#include "perp_ftx.h"
#include "symbol_index.h"

#include <string>

namespace miye::trading::ftx
{

struct PerpFtxApp : public essential::app<PerpFtxApp>
{
    int run(int argc, char** argv)
    {
        cxxopts::Options options("test", "A brief description");

        options.add_options()("b,bar",
                              "Param bar",                                                          //
                              cxxopts::value<std::string>())                                        //
            ("d,date", "trade date", cxxopts::value<std::string>())                                 //
            ("c,config", "config file", cxxopts::value<std::string>()->default_value("config.ini")) //
            ("h,help", "Print usage")                                                               //
            ("v,version", "version information");

        auto option = options.parse(argc, argv);

        if (option.count("help"))
        {
            std::cout << options.help() << std::endl;
            exit(0);
        }

        if (!option.count("date"))
        {
            std::cout << "trade date is requried for logs" << std::endl;
            exit(0);
        }
        std::string date = option["date"].as<std::string>();

        if (option.count("version"))
        {
            std::cout << "perp_ftx version " << GIT_VERSION << "\n";
            exit(0);
        }

        std::string configFile = option["config"].as<std::string>();
        std::cout << "config file:" << configFile << std::endl;
        if (init(date, configFile) != 0)
        {
            return -1;
        }

        loop();
        return 0;
    }

    /*
     * main loop of the program
     */
    int loop()
    {
        while (1)
        {
            perpFtx_.onTimeout(); // TODO:move this to dispatcher
            marketMain.poll();
        }
    }

    int init(std::string date, std::string configFile)
    {
        auto iniFile       = ini::IniFile(configFile);
        auto const logFile = ConfigLoader::getLogFile(date, configFile);

        auto* logger = marketMain.createLogger(logFile);
        if (perpFtx_.init(logger, configFile) != 0)
        {
            logger->info("failed to init perpFtx object");
            return -1;
        }
        if (marketMain.init(configFile, &perpFtx_) != 0)
        {
            logger->info("failed to init market main");
            return -1;
        }
        perpFtx_.context.symbols = marketMain.getSymbols();
        return 0;
    }

    MarketMain<to_underlying(SymbolIndex::NUM_SYMBOLS)> marketMain{};
    ftx::PerpFtx perpFtx_{};
};
} // namespace miye::trading::ftx
