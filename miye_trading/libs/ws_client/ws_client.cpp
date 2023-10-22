#include "ws_client.h"
#include "ws_util/Encoding.h"
#include "ws_util/Time.h"

#include <utility>

// namespace encoding = ws_util::encoding;

namespace miye
{

void WSClient::on_message(ws_util::WS::OnMessageCB cb)
{
    ws.set_on_message_cb(cb);
}

void WSClient::connect() { ws.connect(); }

void WSClient::poll() { ws.poll(); }

std::vector<WSClient::json> WSClient::on_open()
{
    std::vector<json> msgs;

    if (!(wsconfig_.key.empty() || wsconfig_.secret.empty()))
    {
        long ts = ws_util::get_ms_timestamp(ws_util::current_time()).count();
        std::string data = std::to_string(ts) + "websocket_login";
        std::string hmacced =
            ws_util::encoding::hmac(std::string(wsconfig_.secret), data, 32);
        std::string sign = ws_util::encoding::string_to_hex(
            (unsigned char*)hmacced.c_str(), 32);
        json msg = {
            {"op", "login"},
            {"args", {{"key", wsconfig_.key}, {"sign", sign}, {"time", ts}}}};
        if (!wsconfig_.account.empty())
        {
            msg.push_back({"subaccount", wsconfig_.account});
        }
        msgs.push_back(msg);
    }

    for (auto& [symbol, channel] : subscriptions)
    {
        /*
         * in FTX the "symbol" means symbol in normal trading context
         */
        json msg = {
            {"op", "subscribe"}, {"channel", channel}, {"market", symbol}};
        msgs.push_back(msg);
    }

    return msgs;
}

void WSClient::subscribe_orders(std::string symbol)
{
    subscriptions.push_back(std::make_pair(symbol, "orders"));
}

void WSClient::subscribe_orderbook(std::string symbol)
{
    subscriptions.push_back(std::make_pair(symbol, "orderbook"));
}

void WSClient::subscribe_fills(std::string symbol)
{
    subscriptions.push_back(std::make_pair(symbol, "fills"));
}

void WSClient::subscribe_trades(std::string symbol)
{
    subscriptions.push_back(std::make_pair(symbol, "trades"));
}

void WSClient::subscribe_ticker(std::string symbol)
{
    subscriptions.push_back(std::make_pair(symbol, "ticker"));
}

void WSClient::ping() { ws.ping(); }

void WSClient::pong() { ws.pong(); }

} // namespace miye
