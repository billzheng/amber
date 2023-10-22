#include <thread>


#include "coinbase_exchange.h"

#include "../../libs/rings/ring_logger.h"

namespace dinobot { namespace exchanges {

coinbase_exchange::coinbase_exchange(dinobot::lib::configs &c)
    : coinbasews_(nullptr)
    , coinbaserest_(nullptr)
    , coinbase_ws_logger_(nullptr)
//    , coinbase_rest_logger_(nullptr)
    , products_()
{
    std::string uri = c.get_config<std::string>("exchange_ws", "uri");
    std::string path = c.get_config<std::string>("ring_writer_md", "path");
    coinbasews_ = std::make_unique<dinobot::exchanges::coinbase::coinbase_websocket>(0,
                                                                      path,
                                                                      c.get_config<int>("ring_writer_md", "mtu"),
                                                                      c.get_config<int>("ring_writer_md", "elements"),
                                                                      c.get_config<int>("ring_writer_md", "num_readers") );
    uint32_t c_id = coinbasews_->add_connection(uri);

    std::string builder = "";
    for (auto &n: c.get_startup_trade_data<std::vector<std::string>>("coinbasepro", "symbol_list_dash"))
    {
        builder += std::string(",") + std::string("\"") + n + std::string("\"");
        products_.push_back(n);
    }
    builder.erase(0,1); // remove leading ,

    std::string sub = std::string("{\"type\":\"subscribe\",\"product_ids\":[") + builder + std::string("],\"channels\":[\"full\",\"heartbeat\"]}");
    coinbasews_->add_subscription(c_id, sub);

    coinbasews_->start();

    // init everything
    //init();

    // 2 second sleep to make sure we have subscribed
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    coinbase_ws_logger_ = new dinobot::lib::shm::ring_logger(c.get_config<std::string>("ring_writer_md", "path"), 
                                                         c.get_config<std::string>("logs", "path_ws"), 0/*add this to config somehow TODO*/);
    
    logger_thread_ = coinbase_ws_logger_->start_thread();

    coinbaserest_ = std::make_unique<dinobot::lib::rest::rest_client>(c);
}

coinbase_exchange::~coinbase_exchange()
{
    // TODO delete the objects and close ... 
}

void coinbase_exchange::set_finish_time(time_t midnight)
{
    finish_ = midnight;
}

void coinbase_exchange::init()
{
    
}

/* 
 * Start the market data reading ... 
 */
void coinbase_exchange::start()
{
	time_t exittime = time(0);
    time_t booktime = time(0);

    // do shit here ... 

    // 

    while (difftime(finish_, exittime) > 0)
    {
	    exittime = time(0);
	    std::this_thread::sleep_for(std::chrono::milliseconds(500)); 

        //std::cout << "in loop function" << std::endl;
	    //if (! coinbasews_->valid())
	    //    break;
        if (difftime(booktime, exittime) < 0)
        {
            for (auto &p: products_)
            {
                std::string temp = "/products/" + p + "/book" + "?level=3";
                coinbaserest_->send_get_req(temp, p);
            }
            booktime = time(0) + 300; // 5 minutes in the future
        }

	    // here we are checking if we have 
        //
	    if (difftime(finish_, exittime) < 0)
	        break;

    }
}

}} // dinobot::exchanges::coinbase

