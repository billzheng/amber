#pragma once
#include "../../../trading/md_listener.h"
#include "binance_raw_msg.h"
#include "libcore/types/types.hpp"
#include "libcore/utils/number_utils.hpp"
#include "libcore/utils/string_utils.hpp"
#include "libs/json/json.hpp"
#include "libs/logger/logger.hpp"
#include "market_data/book/exchange.h"
#include "market_data/book/order_book.h"
#include <iostream>

namespace miye
{
namespace trading
{
namespace binance
{

template <typename OrderBookStore>
class BinanceMdProcessor
{
  public:
    using json = nlohmann::json;

    explicit BinanceMdProcessor(OrderBookStore& orderBookStore) : orderBookStore_(orderBookStore) {}

    void onMessageCB(const nlohmann::json& j);
    void onSnapshot(const json& j);
    //    void onBookChange(const json& j);
    void onAggTrade(const json& j);
    void onData(const json& j);

    void logBook(const symbol_t& symbol, const OrderBook& book);
    void setMdListener(MDListener* mdListener) { mdListener_ = mdListener; }

    void setLogger(logger::Logger* logger) { logger_ = logger; }

  private:
    logger::Logger* logger() { return this->logger_; }
    logger::Logger* logger_{nullptr};
    OrderBookStore& orderBookStore_;
    MDListener* mdListener_{nullptr};
};

template <typename OrderBookStore>
inline void BinanceMdProcessor<OrderBookStore>::onMessageCB(const json& j)
{
    if (j.contains("stream"))
    {
        auto const& stream = j["stream"];
        auto const fields  = string_utils::split(stream, '@'); // slow code
        assert(fields.size() >= 2);
        if (fields.size() < 2)
        {
            logger()->info("binance::onMessageCB invalid stream:{} rawMsg:{}", stream, j);
            return;
        }
        auto const streamName = fields[1];
        auto const data       = j["data"];
        onData(data);
    }
    else
    {
        logger()->info("binance::onMessageCB {}", j);
        onData(j);
    }
}

template <typename OrderBookStore>
inline void BinanceMdProcessor<OrderBookStore>::onData(const json& j)
{
    auto const& jEventType = j["e"];
    auto const eventType   = fromEventTypeStr(jEventType);
    auto const symbol      = j["s"];
    //    logger()->info(
    //        "binance msg:{} msgType: {}", jEventType,
    //        to_underlying(eventType));
    auto const cid = orderBookStore_.getCid(Exchange::BINANCE, symbol);
    if (eventType == EventType::DepthUpdate)
    {
        onSnapshot(j);

        if (mdListener_)
        {
            mdListener_->onBookChange(cid);
        }
    }
    else if (eventType == EventType::AggTrade)
    {
        logger()->info("binance aggTrade:{}", j);
        onAggTrade(j);

        price_t price;
        quantity_t qty{};
        Side side{};
        uint64_t timestamp{};
        uint64_t tradeId{};
        if (mdListener_)
        {
            mdListener_->onTrade(timestamp, cid, tradeId, side, price, qty, true);
        }
    }
    else if (eventType == EventType::Trade)
    {
        logger()->info("binance trade:{}", j);
        price_t price;
        quantity_t qty{};
        Side side{};
        uint64_t timestamp{};
        uint64_t tradeId{};
        if (mdListener_)
        {
            mdListener_->onTrade(timestamp, cid, tradeId, side, price, qty, true);
        }
    }
    else if (eventType == EventType::BookTicker)
    {
        logger()->info("binance bookTicker:{}", j);

        if (mdListener_)
        {
            mdListener_->onTick(cid);
        }
    }

    // straLogic->run();
}

template <typename OrderBookStore>
inline void BinanceMdProcessor<OrderBookStore>::onSnapshot(const json& j)
{
    // std::cout << "binance::snapshot:" << j << std::endl;
    auto const& jSymbol = j["s"];
    auto const& jAsks   = j["a"];
    auto const& jBids   = j["b"];
    //
    //    logger()->info(
    //        "binance::snapshot symbol:{} asks:{} bids:{}", jSymbol, jAsks,
    //        jBids);

    auto& orderBook         = orderBookStore_.getBook(Exchange::BINANCE, jSymbol);
    const int32_t BookDepth = 5;

    for (auto const& [lvlIdx, level] : jBids.items())
    {
        auto const price = level[0];
        auto const qty   = level[1];

        //        logger()->info(
        //            "binance onBookChange, side bid, process price: {} qty:
        //            {}", price, qty);

        orderBook.setLevel(Side::BUY,
                           BookDepth - number_utils::toInt<int32_t>(lvlIdx) - 1,
                           number_utils::toDouble(price),
                           number_utils::toDouble(qty));
    }

    for (auto const& [lvlIdx, level] : jAsks.items())
    {
        auto const price = level[0];
        auto const qty   = level[1];

        //        logger()->info(
        //            "binance onBookChange, side ask, process price: {} qty:
        //            {}", price, qty);
        orderBook.setLevel(Side::SELL,
                           BookDepth - number_utils::toInt<int32_t>(lvlIdx) - 1,
                           number_utils::toDouble(price),
                           number_utils::toDouble(qty));
    }

    logBook(jSymbol, orderBook);
    //    auto const bbo = orderBook.getBBO();
    //    logger()->info("binance book is valid?{} {} ", orderBook.isValid(),
    //    bbo);
}

template <typename OrderBookStore>
inline void BinanceMdProcessor<OrderBookStore>::onAggTrade(const json& j)
{
    // aggregated tradeId
    auto const tradeId                = j["a"];
    auto const price                  = j["p"];
    auto const qty                    = j["q"];
    const bool wasTheBuyerMarketMaker = j["m"];
    auto const symbol                 = j["s"];
    auto const eventTime              = j["E"];
    auto const side                   = wasTheBuyerMarketMaker ? Side::SELL : Side::BUY;

    logger()->info("binance trade symbol:{} price:{} qty:{} side:{} id:{} tradeTime:{} ",
                   symbol,
                   price,
                   qty,
                   toString(side),
                   tradeId,
                   eventTime);
}

template <typename OrderBookStore>
inline void BinanceMdProcessor<OrderBookStore>::logBook(const std::string& symbol, const OrderBook& book)
{

    logger()->info("quote binance {} {}", symbol, book);
}

} // namespace binance
} // namespace trading
} // namespace miye
