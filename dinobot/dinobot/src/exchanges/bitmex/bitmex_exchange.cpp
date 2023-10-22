#include <thread>


#include "bitmex_exchange.h"

#include "../../libs/rings/ring_logger.h"

namespace dinobot { namespace exchanges {

bitmex_exchange::bitmex_exchange(dinobot::lib::configs &c)
    : bitmexws_(nullptr)
    , bitmexrest_(nullptr)
    , bitmex_ws_logger_(nullptr)
//    , bitmex_rest_logger_(nullptr)
    , products_()
{
    std::string uri = c.get_config<std::string>("exchange_ws", "uri");
    std::string path = c.get_config<std::string>("ring_writer_md", "path");
    bitmexws_ = std::make_unique<dinobot::exchanges::bitmex::bitmex_websocket>(0,
                                                                      path,
                                                                      c.get_config<int>("ring_writer_md", "mtu"),
                                                                      c.get_config<int>("ring_writer_md", "elements"),
                                                                      c.get_config<int>("ring_writer_md", "num_readers") );
    uint32_t c_id = bitmexws_->add_connection(uri);

    for (auto &n: c.get_startup_trade_data<std::vector<std::string>>("bitmex", "symbol_list"))
    {
        std::string sub = "{\"op\":\"subscribe\",\"args\":[\"orderBookL2:" + n + "\"]}";
        bitmexws_->add_subscription(c_id, sub);
        sub = "{\"op\":\"subscribe\",\"args\":[\"trade:" + n + "\"]}";
        bitmexws_->add_subscription(c_id, sub);
        sub = "{\"op\":\"subscribe\",\"args\":[\"quote:" + n + "\"]}";
        bitmexws_->add_subscription(c_id, sub);
        products_.push_back(n);
    }
    std::string sub = "{\"op\":\"subscribe\",\"args\":[\"connected\", \"funding\",\"instrument\", \"insurance\", \"liquidation\"]}";
    bitmexws_->add_subscription(c_id, sub);

    bitmexws_->start();

    // init everything
    //init();

    // 2 second sleep to make sure we have subscribed
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    bitmex_ws_logger_ = new dinobot::lib::shm::ring_logger(c.get_config<std::string>("ring_writer_md", "path"), 
                                                         c.get_config<std::string>("logs", "path_ws"), 0/*add this to config somehow TODO*/);
    
    logger_thread_ = bitmex_ws_logger_->start_thread();

    // rest clients 
    bitmexrest_ = std::make_unique<dinobot::lib::rest::rest_client>(c);
}

bitmex_exchange::~bitmex_exchange()
{
    // TODO delete the objects and close ... 
}

void bitmex_exchange::set_finish_time(time_t midnight)
{
    finish_ = midnight;
}

void bitmex_exchange::init()
{
    
}

/* 
 * Start the market data reading ... 
 */
void bitmex_exchange::start()
{
	time_t exittime = time(0);
    time_t booktime = time(0);

    std::string announce = "/api/v1/announcement";
    bitmexrest_->send_get_req(announce, "announce");

    std::string all_instruments = "/api/v1/instrument?reverse=false";
    bitmexrest_->send_get_req(all_instruments, "all_instruments");

    std::string activeinternal = "/api/v1/instrument/activeIntervals";
    bitmexrest_->send_get_req(all_instruments, "active_internal");

    std::string active = "/api/v1/instrument/activeAndIndices";
    bitmexrest_->send_get_req(active, "activeandindicies");

    std::string composite_index = "/api/v1/instrument/compositeIndex";
    bitmexrest_->send_get_req(all_instruments, "composite_index");

    std::string stats = "/api/v1/stats";
    bitmexrest_->send_get_req(active, "stats");

    while (difftime(finish_, exittime) > 0)
    {
	    exittime = time(0);
	    std::this_thread::sleep_for(std::chrono::milliseconds(500)); 

        if (difftime(booktime, exittime) < 0)
        {
            std::cout << "IN LOOP: " << std::endl;

            std::string funding = "/api/v1/funding";
            bitmexrest_->send_get_req(funding, "funding");

            for (auto &p: products_)
            {
                std::string orderbookl2 = "/api/v1/orderBook/L2?depth=0&symbol=" + p;
                std::string action = p + "_orderbook_l2";
                bitmexrest_->send_get_req(orderbookl2, action);
            }
            booktime = time(0) + 300; // 5 minutes in the future
        }

	    // here we are checking if we have 
        //
	    if (difftime(finish_, exittime) < 0)
	        break;

    }
}

}} // dinobot::exchanges::bitmex

