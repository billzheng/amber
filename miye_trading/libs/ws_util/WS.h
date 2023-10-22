#pragma once

#include "libs/logger/logger.hpp"

#include <functional>
#include <json/json.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_client.hpp>

namespace miye
{
namespace ws_util
{
using json = nlohmann::json;

class WS
{
  public:
    using WSClient = websocketpp::client<websocketpp::config::asio_tls_client>;
    using OnOpenCB = std::function<std::vector<json>()>;
    using OnMessageCB = std::function<void(const json& j)>;
    using OnPingCB = websocketpp::ping_handler;

    WS();
    void configure(std::string _uri, std::string _api_key,
                   std::string _api_secret, std::string _subaccount_name);
    void set_on_open_cb(OnOpenCB open_cb);
    void set_on_message_cb(OnMessageCB message_cb);
    void connect();
    void poll();
    void pong();
    void ping();

    void setLogger(logger::Logger* logger) { logger_ = logger; }

  private:
    logger::Logger* logger() { return logger_; }

  private:
    logger::Logger* logger_{nullptr};
    WSClient wsclient;
    WSClient::connection_ptr connection;
    OnOpenCB on_open_cb;
    OnMessageCB on_message_cb;
    std::string uri;
    std::string api_key;
    std::string api_secret;
    std::string subaccount_name;
};

} // namespace ws_util
} // namespace miye
