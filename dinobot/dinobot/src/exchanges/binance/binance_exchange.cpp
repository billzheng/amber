#include <thread>


#include "binance_exchange.h"

#include "../../libs/rings/ring_logger.h"

namespace dinobot { namespace exchanges {

    //
    // TODO fix this as it is sort of broken 
    //
binance_exchange::binance_exchange(dinobot::lib::configs &c)
    : binancews_(nullptr)
    , binancerest_(nullptr)
    , binance_logger_(nullptr)
{
    std::string uri = c.get_config<std::string>("exchange_ws", "uri");
    std::string path = c.get_config<std::string>("ring_writer_md", "path");
    binancews_ = std::make_unique<dinobot::exchanges::binance::binance_websocket>(0,
                                                                      path,
                                                                      c.get_config<int>("ring_writer_md", "mtu"),
                                                                      c.get_config<int>("ring_writer_md", "elements"),
                                                                      c.get_config<int>("ring_writer_md", "num_readers") );

    uint32_t c_id = binancews_->add_connection(uri);
    binancews_->add_stream(c_id, c.get_startup_trade_data<std::string>("binance", "streams", "trade") );
    binancews_->add_stream(c_id, c.get_startup_trade_data<std::string>("binance", "streams", "ticker") );
    binancews_->add_stream(c_id, c.get_startup_trade_data<std::string>("binance", "streams", "depth5") );
    binancews_->add_stream(c_id, c.get_startup_trade_data<std::string>("binance", "streams", "kline5") );
    binancews_->add_stream(c_id, c.get_startup_trade_data<std::string>("binance", "streams", "depth") );
    
    binancews_->start();

    // init everything
    init();

    // 2 second sleep to make sure we have subscribed
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    binance_logger_ = new dinobot::lib::shm::ring_logger(c.get_config<std::string>("ring_writer_md", "path"), 
                                                         c.get_config<std::string>("logs", "path"), 0/*add this to config somehow TODO*/);
    logger_thread_ = binance_logger_->start_thread();

    // rest clients 
    /*
    binancerest_ = new dinobot::lib::rest::rest_client(c.get_config<std::string>("exchange_rest", "host"), 
                                                       c.get_config<unsigned int>("exchange_rest", "port"), 
                                                       c.get_config<std::string>("exchange_rest", "ca"));
    */
    binancerest_ = std::make_unique<dinobot::lib::rest::rest_client>(c);
}

binance_exchange::~binance_exchange()
{
    // TODO delete the objects and close ... 
}

void binance_exchange::set_finish_time(time_t midnight)
{
    finish_ = midnight;
}

void binance_exchange::init()
{
    
}

/* 
 * Start the market data reading ... 
 */
void binance_exchange::start()
{
	time_t exittime = time(0);

    while (difftime(finish_, exittime) > 0)
    {
	    exittime = time(0);
	    std::this_thread::sleep_for(std::chrono::milliseconds(2000)); 


	    // here we are checking if we have 
        //
	    if (difftime(finish_, exittime) < 0)
	        break;

    }
}

}} // dinobot::exchanges::binance

