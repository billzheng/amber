#pragma once

#include "libs/logger/logger.hpp"
#include <json/json.hpp>
#include <ws_util/WS.h>

#include <string>
#include <vector>

namespace miye
{
struct WSConfig
{
    std::string uri;
    std::string key;
    std::string secret;
    std::string account;
    std::vector<std::string> symbols;
};

class WSClient
{
  public:
    using json = nlohmann::json;

  public:
    //    explicit WSClient(WSConfig wsconfig) : wsconfig_(wsconfig)
    //    {
    //        ws.configure(
    //            wsconfig_.uri, wsconfig_.key, wsconfig_.secret,
    //            wsconfig_.account);
    //        ws.set_on_open_cb([this]() { return this->on_open(); });
    //    }

    WSClient() = default;

    void on_message(ws_util::WS::OnMessageCB cb);
    void connect();
    void poll();
    void ping();
    void pong();
    std::vector<json> on_open();

    /*
     * in FTX the "market" means symbol in normal trading context
     */
    void subscribe_orders(std::string symbol);
    void subscribe_orderbook(std::string symbol);
    void subscribe_fills(std::string symbol);
    void subscribe_trades(std::string symbol);
    void subscribe_ticker(std::string symbol);

    void setLogger(logger::Logger* logger)
    {
        this->logger_ = logger;
        ws.setLogger(logger);
    }

    int32_t init(logger::Logger* logger, WSConfig wsconfig)
    {
        setLogger(logger);

        ws.setLogger(logger);
        wsconfig_ = wsconfig;
        ws.configure(
            wsconfig_.uri, wsconfig_.key, wsconfig_.secret, wsconfig_.account);
        ws.set_on_open_cb([this]() { return this->on_open(); });
        return 0;
    }

  private:
    logger::Logger* logger_{nullptr};

    std::vector<std::pair<std::string, std::string>> subscriptions;
    ws_util::WS::OnMessageCB message_cb;
    ws_util::WS ws;

    WSConfig wsconfig_;
};

} // namespace miye
