#pragma once
#include "ftx_md_processor.h"
#include "libcore/types/types.hpp"
#include "libs/logger/logger.hpp"

#include <json/json.hpp>
#include <libs/ws_client/ws_client.h>

namespace miye
{
namespace trading
{
namespace ftx
{

template <typename OrderBookStore>
class WSClientFtx
{
  public:
    using json = nlohmann::json;

  public:
    WSClientFtx(OrderBookStore& bookStore) : ftxMdProcessor_(bookStore) {}

    void setMsgProcessor()
    {
        wsClient_.on_message(
            std::bind(&FtxMdProcessor<OrderBookStore>::onMessageCB,
                      &ftxMdProcessor_,
                      std::placeholders::_1));
    }

    void connect() { wsClient_.connect(); }
    void poll() { return wsClient_.poll(); }

    void subscribeOrderbook(std::string market)
    {
        wsClient_.subscribe_orderbook(market);
    }

    void subscribeOrders(std::string market)
    {
        wsClient_.subscribe_orders(market);
    }

    void subscribeFills(std::string market)
    {
        wsClient_.subscribe_fills(market);
    }

    void subscribeTrades(std::string market)
    {
        wsClient_.subscribe_trades(market);
    }

    void subscribeTicker(std::string market)
    {
        wsClient_.subscribe_ticker(market);
    }

    void ping() { wsClient_.ping(); }
    void setMdListener(MDListener* mdListener)
    {
        ftxMdProcessor_.setMdListener(mdListener);
    }

    void setLogger(logger::Logger* logger)
    {
        this->logger_ = logger;
        ftxMdProcessor_.setLogger(logger);
        wsClient_.setLogger(logger);
    }
    int32_t init(logger::Logger* logger, WSConfig wsconfig)
    {
        logger_ = logger;
        ftxMdProcessor_.setLogger(logger);
        setMsgProcessor();
        wsClient_.init(logger, wsconfig);
        return 0;
    }

  private:
    logger::Logger* logger_{nullptr};
    FtxMdProcessor<OrderBookStore> ftxMdProcessor_;
    WSClient wsClient_;
};

} // namespace ftx
} // namespace trading
} // namespace miye
