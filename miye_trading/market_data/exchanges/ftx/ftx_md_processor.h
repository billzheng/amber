#pragma once
#include "../../../trading/md_listener.h"
#include "ftx_raw_msg.h"
#include "libcore/types/types.hpp"
#include "libcore/utils/number_utils.hpp"
#include "libs/json/json.hpp"
#include "libs/logger/logger.hpp"
#include "market_data/book/order_book.h"
#include <chrono>
#include <iostream>

namespace miye
{
namespace trading
{
namespace ftx
{

template <typename OrderBookStore>
class FtxMdProcessor
{
  public:
    using json = nlohmann::json;

    explicit FtxMdProcessor(OrderBookStore& orderBookStore) : orderBookStore_(orderBookStore)
    {

        //        // TODO:pass rollWindowNs from config
        //        // 5min
        //        uint64_t const rollWindowNs = 300'000'000'000;
        //        tradeFlow_.init(rollWindowNs);
    }

    void onMessageCB(const nlohmann::json& j);

    void onSnapshot(const json& j);
    void onBookChange(const json& j);
    void onTrade(const json& j);

    void logBook(const std::string& symbol, const OrderBook& book);
    void setMdListener(MDListener* mdListener) { mdListener_ = mdListener; }
    void setLogger(logger::Logger* logger) { logger_ = logger; }

  private:
    logger::Logger* logger() { return this->logger_; }
    logger::Logger* logger_{nullptr};
    OrderBookStore& orderBookStore_;
    MDListener* mdListener_{nullptr};
};

template <typename OrderBookStore>
inline void FtxMdProcessor<OrderBookStore>::onMessageCB(const json& j)
{
    //    std::cout << "ftx::onMessageCB " << j << std::endl;
    // logger()->info("ftx::onMessageCB {}", j);

    auto const& jType    = j["type"];
    auto const& jChannel = j["channel"];
    auto const type      = fromMsgTypeString(jType);
    auto const channel   = fromChannelStr(jChannel);
    //    logger()->info(
    //        "ftx msg:{} msgType: {}", toString(type), to_underlying(type));

    if (type == MsgType::PARTIAL && channel == Channel::ORDERBOOK)
    {
        onSnapshot(j);
    }
    else if (type == MsgType::UPDATE && channel == Channel::ORDERBOOK)
    {
        onBookChange(j);
    }
    else if (type == MsgType::UPDATE && channel == Channel::TRADES)
    {
        onTrade(j);
    }
}

template <typename OrderBookStore>
inline void FtxMdProcessor<OrderBookStore>::onTrade(const json& j)
{
    logger()->info("ftx trade:{}", j);

    const std::string jSymbol = j["market"];
    auto const& type          = fromMsgTypeString(j["type"]);
    auto const& jTrades       = j["data"];

    auto const cid  = orderBookStore_.getCid(Exchange::FTX, jSymbol);
    auto& orderBook = orderBookStore_.getBook(cid);

    std::vector<trade_t> trades;

    /*
     * sometimes trades contains both buy/sell trades
     * FTX seems put buy trades in the front of the packet follow up sell trades
     */
    auto tradeNum = jTrades.size();
    for (auto const& trade : jTrades)
    {
        auto const& side        = fromSideStr(trade["side"]);
        auto const& id          = trade["id"];
        auto const& liquidation = trade["liquidation"];
        auto const& price       = trade["price"];
        auto const& qty         = trade["size"];
        auto const& jTime       = trade["time"];
        uint64_t timestamp      = 123;

        logger()->info("ftx trade symbol:{} id:{} price:{} qty:{} side:{} "
                       "tradeTime:{} liquidation:{}",
                       jSymbol,
                       id,
                       price,
                       qty,
                       trade["side"],
                       jTime,
                       liquidation);
        trades.emplace_back(trade_t{price, qty, side});
        orderBook.setLastTrade(timestamp, id, side, price, qty);
        tradeNum--;
        if (mdListener_)
        {
            mdListener_->onTrade(timestamp, cid, id, side, price, qty, tradeNum == 0);
        }
    }

    assert(tradeNum == 0);
    // auto const signal = tradeFlow_.onTrade(timestamp, trades);
    // logger()->info("trade flow:{:0.4f}", signal.getValue());
}

template <typename OrderBookStore>
inline void FtxMdProcessor<OrderBookStore>::onSnapshot(const json& j)
{
    // std::cout << "ftx::snapshot:" << j << std::endl;
    auto const& jSymbol = j["market"];
    auto const& jData   = j["data"];

    auto const& jTime   = jData["time"];
    auto const& jAction = jData["action"];
    auto const& jAsks   = jData["asks"];
    auto const& jBids   = jData["bids"];

    logger()->info("ftx::snapshot symbol:{} time:{} asks:{} bids:{}", jSymbol, jTime, jAsks, jBids);
    //    std::cout << "ftx::snapshot symbol:" << jSymbol << " time:" << jTime
    //              << " \nasks:" << jAsks << " \nbids:" << jBids << std::endl;

    auto const cid  = orderBookStore_.getCid(Exchange::FTX, jSymbol);
    auto& orderBook = orderBookStore_.getBook(Exchange::FTX, jSymbol);

    for (auto const& [lvlIdx, level] : jBids.items())
    {
        auto const price = level[0];
        auto const qty   = level[1];
        // std::cout << "process price:" << price << " qty:" << qty <<
        // std::endl;
        orderBook.setOrInsertLevel(Side::BUY, price, qty);
    }

    for (auto const& [lvlIdx, level] : jAsks.items())
    {
        auto const price = level[0];
        auto const qty   = level[1];
        orderBook.setOrInsertLevel(Side::SELL, price, qty);
    }

    // logger()->info("ftx onSnapshot finished");
    logBook(jSymbol, orderBook);

    if (mdListener_)
    {
        mdListener_->onSnapshotFinished(cid);
    }
    // TODO: checksum check
    // orderBook.print();
}

template <typename OrderBookStore>
inline void FtxMdProcessor<OrderBookStore>::onBookChange(const json& j)
{
    auto const& jSymbol = j["market"];
    auto const& jData   = j["data"];

    auto const& jTime   = jData["time"];
    auto const& jAction = jData["action"];
    auto const& jAsks   = jData["asks"];
    auto const& jBids   = jData["bids"];

    auto const cid  = orderBookStore_.getCid(Exchange::FTX, jSymbol);
    auto& orderBook = orderBookStore_.getBook(Exchange::FTX, jSymbol);

    for (auto const& [lvlIdx, level] : jBids.items())
    {
        auto const price = level[0];
        auto const qty   = level[1];

        //        logger()->info("ftx onBookChange, side bid, process price: {}
        //        qty: {}",
        //                       price,
        //                       qty);
        if (qty > 0.0)
        {
            orderBook.setOrInsertLevel(Side::BUY, price, qty);
        }
        else
        {
            orderBook.removeLevel(Side::BUY, price);
        }
    }

    for (auto const& [lvlIdx, level] : jAsks.items())
    {
        auto const price = level[0];
        auto const qty   = level[1];

        //        logger()->info("ftx onBookChange, side ask, process price: {}
        //        qty: {}",
        //                       price,
        //                       qty);
        if (qty > 0.0)
        {
            orderBook.setOrInsertLevel(Side::SELL, price, qty);
        }
        else
        {
            orderBook.removeLevel(Side::SELL, price);
        }
    }
    logBook(jSymbol, orderBook);

    if (mdListener_)
    {
        mdListener_->onBookChange(cid);
    }
}

template <typename OrderBookStore>
inline void FtxMdProcessor<OrderBookStore>::logBook(const std::string& symbol, const OrderBook& book)
{
    logger()->info("quote ftx {} {}", symbol, book);
}

} // namespace ftx
} // namespace trading
} // namespace miye
