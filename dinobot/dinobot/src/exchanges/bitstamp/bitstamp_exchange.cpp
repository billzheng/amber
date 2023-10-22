#include <thread>


#include "bitstamp_exchange.h"

#include "../../libs/rings/ring_logger.h"

namespace dinobot { namespace exchanges {

bitstamp_exchange::bitstamp_exchange(dinobot::lib::configs &c)
    : bitstampws_(nullptr)
    , bitstamprest_(nullptr)
    , bitstamp_ws_logger_(nullptr)
    , products_()
{
    for (auto &n: c.get_startup_trade_data<std::vector<std::string>>("bitstamp", "symbol_list"))
        products_.push_back(n);

    std::string uri = c.get_config<std::string>("exchange_ws", "uri");
    std::string path = c.get_config<std::string>("ring_writer_md", "path");
    bitstampws_ = std::make_unique<dinobot::exchanges::bitstamp::bitstamp_websocket>(0,
                                                                      path,
                                                                      c.get_config<int>("ring_writer_md", "mtu"),
                                                                      c.get_config<int>("ring_writer_md", "elements"),
                                                                      c.get_config<int>("ring_writer_md", "num_readers") );

    // the /app/KEY is the bitstamp key we need to link to the channel
    std::string t = uri + "/app/de504dc5763aeef9ff52?client=linux-dino&version=0.0.9&protocol=7";

    uint32_t c_id = bitstampws_->add_connection(t);

    for (auto &p : products_)
    {
        std::string lt = std::string("{\"data\": {\"channel\": \"live_trades_" + p + "\"}, \"event\": \"pusher:subscribe\"}");
        bitstampws_->add_subscription(c_id, lt);
        
        std::string ob = std::string("{\"data\": {\"channel\": \"order_book_" + p + "\"}, \"event\": \"pusher:subscribe\"}");
        bitstampws_->add_subscription(c_id, ob);

        std::string fob = std::string("{\"data\": {\"channel\": \"diff_order_book_" + p + "\"}, \"event\": \"pusher:subscribe\"}");
        bitstampws_->add_subscription(c_id, fob);

        std::string lo = std::string("{\"data\": {\"channel\": \"live_orders_" + p + "\"}, \"event\": \"pusher:subscribe\"}");
        bitstampws_->add_subscription(c_id, lo);
    }

    bitstampws_->start();

    // init everything
    //init();

    // 2 second sleep to make sure we have subscribed
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    bitstamp_ws_logger_ = new dinobot::lib::shm::ring_logger(c.get_config<std::string>("ring_writer_md", "path"), 
                                                         c.get_config<std::string>("logs", "path_ws"), 0/*add this to config somehow TODO*/);
    
    logger_thread_ = bitstamp_ws_logger_->start_thread();

    // rest clients 
    bitstamprest_ = std::make_unique<dinobot::lib::rest::rest_client>(c);
}

bitstamp_exchange::~bitstamp_exchange()
{
    // TODO delete the objects and close ... 
}

void bitstamp_exchange::set_finish_time(time_t midnight)
{
    finish_ = midnight;
}

void bitstamp_exchange::init()
{
    
}

/* 
 * Start the market data reading ... 
 */
void bitstamp_exchange::start()
{
	time_t exittime = time(0);

    std::string pairs = "/api/v2/trading-pairs-info/";
    bitstamprest_->send_get_req(pairs, "pairs");
	std::this_thread::sleep_for(std::chrono::seconds(1)); // this will rate limit us to 600/10 minutes

    std::string exchangerate = "/api/eur_usd/";
    bitstamprest_->send_get_req(exchangerate, "exchange_rate");
	std::this_thread::sleep_for(std::chrono::seconds(1)); // this will rate limit us to 600/10 minutes

    while (difftime(finish_, exittime) > 0)
    {
	    exittime = time(0);

        for (auto &p: products_)
        {
            std::string tickers = "/api/v2/ticker/" + p + "/";
            std::string tickers_str = p + "_tickers";
            bitstamprest_->send_get_req(tickers, tickers_str);
            std::this_thread::sleep_for(std::chrono::seconds(1)); // this will rate limit us to 600/10 minutes

            std::string hourlyticker = "/api/v2/ticker_hour/" + p + "/";
            std::string hourlyticker_str = p + "_hourlyticker";
            bitstamprest_->send_get_req(hourlyticker, hourlyticker_str);
            std::this_thread::sleep_for(std::chrono::seconds(1)); // this will rate limit us to 600/10 minutes

            std::string orderbook = "/api/v2/order_book/" + p + "/?group=2";
            std::string orderbook_str = p + "_orderbook";
            bitstamprest_->send_get_req(orderbook, orderbook_str);
            std::this_thread::sleep_for(std::chrono::seconds(1)); // this will rate limit us to 600/10 minutes

            std::string transactions = "/api/v2/order_book/" + p + "/?time=minute";
            std::string transactions_str = p + "_transactions";
            bitstamprest_->send_get_req(transactions, transactions_str);

            std::this_thread::sleep_for(std::chrono::seconds(1)); // this will rate limit us to 600/10 minutes
        }
    }
}

}} // dinobot::exchanges::bitstamp

