#pragma once
#include "binance_md_processor.h"
#include "market_data/book/order_book.h"
#include <libs/ws_client/ws_client.h>

#include <json/json.hpp>

namespace miye
{
namespace trading
{
namespace binance
{

template <typename OrderBookStore>
class WSClientBinance
{
  public:
    using json = nlohmann::json;

  public:
    explicit WSClientBinance(OrderBookStore& orderBookStore)
        : binanceMdProcessor_(orderBookStore)
    {
    }

    void setMsgProcessor()
    {
        wsClient_.on_message(
            std::bind(&BinanceMdProcessor<OrderBookStore>::onMessageCB,
                      &binanceMdProcessor_,
                      std::placeholders::_1));
    }

    void connect() { wsClient_.connect(); }

    void poll() { return wsClient_.poll(); }

    void setMdListener(MDListener* mdListener)
    {
        binanceMdProcessor_.setMdListener(mdListener);
    }
    void setLogger(logger::Logger* logger)
    {
        this->logger_ = logger;
        wsClient_.setLogger(logger);
        binanceMdProcessor_.setLogger(logger);
    }
    int32_t init(logger::Logger* logger, WSConfig wsconfig)
    {
        logger_ = logger;
        binanceMdProcessor_.setLogger(logger);
        setMsgProcessor();
        wsClient_.init(logger, wsconfig);
        return 0;
    }

  private:
    logger::Logger* logger_{nullptr};
    MDListener* mdListener_{nullptr};
    BinanceMdProcessor<OrderBookStore> binanceMdProcessor_;
    WSClient wsClient_;
};

} // namespace binance
} // namespace trading
} // namespace miye
