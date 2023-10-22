#include <thread>

#include "deribit_exchange.h"

#include "../../libs/rings/ring_logger.h"

namespace dinobot { namespace exchanges {

deribit_exchange::deribit_exchange(dinobot::lib::configs &c)
    : deribitws_(nullptr)
    , deribitrest_(nullptr)
    , deribit_ws_logger_(nullptr)
//    , deribit_rest_logger_(nullptr)
    , products_()
{
    std::string uri = c.get_config<std::string>("exchange_ws", "uri");
    std::string path = c.get_config<std::string>("ring_writer_md", "path");
    deribitws_ = std::make_unique<dinobot::exchanges::deribit::deribit_websocket>(0,
                                                                      path,
                                                                      c.get_config<int>("ring_writer_md", "mtu"),
                                                                      c.get_config<int>("ring_writer_md", "elements"),
                                                                      c.get_config<int>("ring_writer_md", "num_readers") );
    uint32_t c_id = deribitws_->add_connection(uri);

    
    for (auto &n: c.get_startup_trade_data<std::vector<std::string>>("deribit", "symbol_list")) // TODO find out which dict i am using here  
    {
        products_.push_back(n); 
    }
    
    // using the rest interface for the time being since my websocket can't deal with ad hoc messages yet
    deribitws_->add_subscription(c_id, "{\"action\": \"/api/v1/public/time\"}");
    //deribitws_->add_subscription(c_id, "{\"action\": \"/api/v1/public/setheartbeat\", \"arguments\": {\"interval\": 60}}");
    //deribitws_->add_subscription(c_id, "{\"action\": \"/api/v1/public/cancelheartbeat\"}");
    deribitws_->add_subscription(c_id, "{\"action\": \"/api/v1/public/test\"}");
    //deribitws_->add_subscription(c_id, "{\"action\": \"/api/v1/public/ping\"}");
    deribitws_->add_subscription(c_id, "{\"action\": \"/api/v1/public/getinstruments\"}");
    deribitws_->add_subscription(c_id, "{\"action\": \"/api/v1/public/getcurrencies\"}");
    deribitws_->add_subscription(c_id, "{\"action\": \"/api/v1/public/index\"}");
    deribitws_->add_subscription(c_id, "{\"action\": \"/api/v1/public/getorderbook\",\"arguments\": {\"instrument\": \"BTC-23FEB18\"}}");
    deribitws_->add_subscription(c_id, "{\"action\": \"/api/v1/public/getlasttrades\", \"arguments\":{\"instrument\":\"all\"}}");
    deribitws_->add_subscription(c_id, "{\"action\": \"/api/v1/public/getsummary\", \"arguments\":{\"instrument\":\"all\"}}");
    deribitws_->add_subscription(c_id, "{\"action\": \"/api/v1/public/stats\"}");
    deribitws_->add_subscription(c_id, "{\"action\": \"/api/v1/public/getannouncements\"}");
    deribitws_->add_subscription(c_id, "{\"action\": \"/api/v1/public/getlastsettlements\", \"arguments\":{\"instrument\":\"all\"}}");
    // 
    std::string testt ="{\"action\": \"/api/v1/public/time\"}"; 
    deribitws_->send(testt);

    deribitws_->start();

    // 2 second sleep to make sure we have subscribed
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    deribit_ws_logger_ = new dinobot::lib::shm::ring_logger(c.get_config<std::string>("ring_writer_md", "path"), 
                                                         c.get_config<std::string>("logs", "path_ws"), 0/*add this to config somehow TODO*/);
    logger_thread_ = deribit_ws_logger_->start_thread();

    // rest clients 
    deribitrest_ = std::make_unique<dinobot::lib::rest::rest_client>(c);
}

deribit_exchange::~deribit_exchange()
{
    // TODO delete the objects and close ... 
}

void deribit_exchange::set_finish_time(time_t midnight)
{
    finish_ = midnight;
}

void deribit_exchange::init()
{
    
}

/* 
 * Start the market data reading ... 
 */
void deribit_exchange::start()
{
	time_t exittime = time(0);
    constexpr int rate_limit = 200; // 200/sec (rolling or bucketed?? )
    int rolling_win = 1;
    time_t future_time = time(0) + 300; // 5 minutes

    std::string time = "/api/v1/public/time";
    deribitrest_->send_get_req(time, "time");
    rolling_win++;

    std::string test = "/api/v1/public/test";
    deribitrest_->send_get_req(test, "test");
    rolling_win++;

    std::string ping = "/api/v1/public/ping";
    deribitrest_->send_get_req(ping, "ping");
    rolling_win++;

    std::string instruments = "/api/v1/public/getinstruments";
    deribitrest_->send_get_req(instruments, "instruments");
    rolling_win++;

    std::string currencies = "/api/v1/public/getcurrencies";
    deribitrest_->send_get_req(currencies, "currencies");
    rolling_win++;

    while (difftime(finish_, exittime) > 0)
    {
        if (rolling_win > rate_limit - 50)
        {
            rolling_win = 0;
            //small sleep to not hit the rate limit
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

	    exittime = ::time(0);

        if (difftime(exittime, future_time) > 0)
        {
            std::string index = "/api/v1/public/index";
            deribitrest_->send_get_req(index, "index");
            rolling_win++;

            std::string summary = "/api/v1/public/getsummary?instrument=all";
            deribitrest_->send_get_req(summary, "summary");
            rolling_win++;

            std::string stats = "/api/v1/public/stats";
            deribitrest_->send_get_req(stats, "stats");
            rolling_win++;

            std::string announce = "/api/v1/public/getannouncements";
            deribitrest_->send_get_req(announce, "announce");
            rolling_win++;

            future_time = exittime + 300;
        }

        std::string settlements = "/api/v1/public/getlastsettlements?instrument=all&count=1000";
        deribitrest_->send_get_req(settlements, "settlements");
        rolling_win++;

        for (auto &p: products_)
        {
            std::string orderbook = "/api/v1/public/getorderbook?instrument=" + p + "&depth=50";
            deribitrest_->send_get_req(orderbook, "orderbook");
            rolling_win++;
        }

        std::string trades = "/api/v1/public/getlasttrades?instrument=all&count=1000";
        deribitrest_->send_get_req(trades, "trades");
        rolling_win++;

    }
}

}} // dinobot::exchanges::deribit

