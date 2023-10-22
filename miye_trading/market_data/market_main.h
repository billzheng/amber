#pragma once

#include "../trading/md_listener.h"
#include "libcore/utils/string_utils.hpp"
#include "libs/inifile/inicpp.h"
#include "libs/logger/logger.hpp"
#include "market_data/book/order_book_store.hpp"
#include "market_data/exchanges/binance/ws_client_binance.h"
#include "market_data/exchanges/ftx/ws_client_ftx.h"

#include <stdint.h>

namespace miye
{
namespace trading
{

template <size_t SYM_SIZE>
struct MarketMain
{
    using OrderBookStore_t = OrderBookStore<SYM_SIZE>;
    static const constexpr int32_t BinanceBookDepth{5};

    int32_t poll()
    {
        binanceWSClient.poll();
        ftxWSClient.poll();
        return 0;
    }

    logger::Logger* createLogger(std::string logFile)
    {
        std::cout << "log path:" << logFile << std::endl;
        logger_ = logger::createLogger(logFile);
        logger_->info("logger path:{}", logFile);
        return logger_.get();
    }

    int32_t init(std::string configFile, MDListener* mdListener)
    {
        auto iniFile           = ini::IniFile(configFile);
        std::string symbolList = iniFile["global"]["symbol_list"].as<std::string>();
        auto const symbols     = string_utils::split(symbolList, ',');
        assert(symbols.size() > 0);
        if (bookStore_.getSymbolNum() != symbols.size())
        {
            logger()->critical("global symbol_list does not match book num in bookstore");
            return -1;
        }

        bookStore_.setSymbols(symbols);

        /*
         * FTX is defaulted to full bookdepth
         * Binance book depth is decided by stream param
         */
        std::cout << "init binance book to depth:" << BinanceBookDepth << std::endl;
        bookStore_.initBook(trading::Exchange::BINANCE, BinanceBookDepth);

        initBinanceMd(configFile, mdListener);
        initFtxMd(configFile, mdListener);

        binanceWSClient.connect();
        ftxWSClient.connect();

        return 0;
    }

    int32_t initFtxMd(std::string configFile, MDListener* mdListener)
    {
        auto iniFile          = ini::IniFile(configFile);
        const std::string uri = iniFile["markets.ftx"]["uri"].as<std::string>();
        WSConfig wsconfig{uri};

        ftxWSClient.init(logger_.get(), wsconfig);
        ftxWSClient.setMdListener(mdListener);

        auto const ftx_symbol = iniFile["markets.ftx"]["symbol_list"].as<std::string>();

        ftxWSClient.subscribeOrderbook(ftx_symbol);
        logger_->info("subscribe to ftx orderbook symbol:{}", ftx_symbol);
        ftxWSClient.subscribeTrades(ftx_symbol);
        logger_->info("subscribe to ftx trades symbol:{}", ftx_symbol);
        ftxWSClient.subscribeTicker(ftx_symbol);
        logger_->info("subscribe to ftx ticker symbol:{}", ftx_symbol);

        return 0;
    }

    int32_t initBinanceMd(std::string configFile, MDListener* mdListener)
    {
        auto iniFile      = ini::IniFile(configFile);
        auto const symbol = iniFile["markets.binance"]["symbol_list"].as<std::string>();
        WSConfig wsconfig{"wss://fstream.binance.com:443/stream?streams=" + symbol + "@depth" +
                          std::to_string(BinanceBookDepth) + "@0ms/" + symbol + "@aggTrade/" + symbol +
                          "@markPrice@1s"};

        binanceWSClient.init(logger_.get(), wsconfig);
        binanceWSClient.setMdListener(mdListener);
        return 0;
    }

    std::vector<std::string> getSymbols() const { return bookStore_.getSymbols(); }

    logger::Logger* logger() { return logger_.get(); }

  private:
    std::shared_ptr<logger::Logger> logger_;
    OrderBookStore_t bookStore_;

    binance::WSClientBinance<OrderBookStore_t> binanceWSClient{bookStore_};
    ftx::WSClientFtx<OrderBookStore_t> ftxWSClient{bookStore_};
};

} // namespace trading
} // namespace miye
