#include "../ws_util/WS.h"

namespace miye
{
namespace ws_util
{

WS::WS()
{
    wsclient.set_access_channels(websocketpp::log::alevel::none);
    wsclient.set_error_channels(websocketpp::log::elevel::fatal);

    wsclient.init_asio();

    wsclient.set_open_handler([this](websocketpp::connection_hdl hdl) {
        auto subscriptions = on_open_cb();
        for (auto& subscription : subscriptions)
        {
            // std::cout << "subscription:" << subscription << std::endl;
            logger_->info("subscription:{}", subscription);
            wsclient.send(
                hdl, subscription.dump(), websocketpp::frame::opcode::text);
        }
    });

    wsclient.set_message_handler(
        [this](websocketpp::connection_hdl, WSClient::message_ptr msg) {
            json j = json::parse(msg->get_raw_payload().c_str());
            on_message_cb(j);
        });

    wsclient.set_close_handler([this](websocketpp::connection_hdl) {
        std::cout << "connection closed" << std::endl;
        logger_->critical(uri + " connection closed");
    });

    wsclient.set_pong_handler(
        [this](websocketpp::connection_hdl, std::string payload) {
            logger_->info(uri + " pong msg received, payload:{}", payload);
        });

    wsclient.set_ping_handler(
        [this](websocketpp::connection_hdl, std::string payload) {
            logger_->info(uri + " ping msg received, paylaod:{}", payload);
            this->pong();
            return true;
        });

    wsclient.set_interrupt_handler(
        [this](websocketpp::connection_hdl) { throw "Interrupt handler"; });

    wsclient.set_fail_handler(
        [this](websocketpp::connection_hdl) { throw "Fail handler"; });

    wsclient.set_tls_init_handler([](websocketpp::connection_hdl) {
        return websocketpp::lib::make_shared<boost::asio::ssl::context>(
            boost::asio::ssl::context::tlsv12);
    });
}

void WS::configure(std::string _uri, std::string _api_key,
                   std::string _api_secret, std::string _subaccount_name)
{
    uri = _uri;
    api_key = _api_key;
    api_secret = _api_secret;
    subaccount_name = _subaccount_name;
}

void WS::set_on_open_cb(OnOpenCB open_cb) { on_open_cb = open_cb; }

void WS::set_on_message_cb(OnMessageCB message_cb)
{
    on_message_cb = message_cb;
}

void WS::connect()
{
    websocketpp::lib::error_code ec;

    std::cout << "connect to uri:" << uri << " api_key:" << api_key
              << " api_secret:" << api_secret
              << " subaccount_name:" << subaccount_name << std::endl;

    connection = wsclient.get_connection(uri, ec);
    if (ec)
    {
        std::string err =
            "Could not create connection because: " + ec.message() + "\n";
        throw err;
    }
    wsclient.connect(connection);
    // wsclient.run();
    //    while (1)
    //    {
    //        wsclient.poll();
    //    }
}

void WS::ping()
{
    logger()->info("sending ping to uri:{}", uri);
    wsclient.ping(connection, "ping");
}
void WS::pong()
{
    logger()->info("sending pong to uri:{}", uri);
    wsclient.pong(connection, "pong");
}

void WS::poll() { wsclient.poll(); }

} // namespace ws_util
} // namespace miye
