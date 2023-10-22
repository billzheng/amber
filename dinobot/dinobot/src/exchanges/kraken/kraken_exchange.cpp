#include <thread>

#include "kraken_exchange.h"

#include "../../libs/rings/ring_logger.h"

namespace dinobot { namespace exchanges {

kraken_exchange::kraken_exchange(dinobot::lib::configs &c)
    : krakenrest_(nullptr)
    , kraken_ws_logger_(nullptr)
    , products_()
    , products_str_("")
{
    std::string uri = c.get_config<std::string>("exchange_ws", "uri");
    std::string path = c.get_config<std::string>("ring_writer_md", "path");
    

    for (auto &n: c.get_startup_trade_data<std::vector<std::string>>("kraken", "symbol_list")) // TODO find out which dict i am using here  
    {
        products_.push_back(n); 
        products_str_ += n + ',';
    } 
    products_str_.pop_back();
    
    // 2 second sleep to make sure we have subscribed
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    kraken_ws_logger_ = new dinobot::lib::shm::ring_logger(c.get_config<std::string>("ring_writer_md", "path"), 
                                                         c.get_config<std::string>("logs", "path_ws"), 0/*add this to config somehow TODO*/);
    logger_thread_ = kraken_ws_logger_->start_thread();

    // rest clients 
    krakenrest_ = std::make_unique<dinobot::lib::rest::rest_client>(c);
}

kraken_exchange::~kraken_exchange()
{
    // TODO delete the objects and close ... 
}

void kraken_exchange::set_finish_time(time_t midnight)
{
    finish_ = midnight;
}

void kraken_exchange::init()
{
    
}

/* 
 * Start the market data reading ... 
 */
void kraken_exchange::start()
{
	time_t exittime = time(0);

    std::string timeval = "/0/public/Time";
    krakenrest_->send_get_req(timeval, "time");

    std::string assets = "/0/public/Assets";
    krakenrest_->send_get_req(assets, "assets");

    std::string assetpairs = "/0/public/AssetPairs";
    krakenrest_->send_get_req(assetpairs, "assetpairs");

    std::this_thread::sleep_for(std::chrono::seconds(10)); // wil lget us back to api call of zero 

    while (difftime(finish_, exittime) > 0)
    {
        std::string ticker = "/0/public/Ticker?" + products_str_;
        krakenrest_->send_get_req(ticker, "ticker");
        std::this_thread::sleep_for(std::chrono::seconds(3));

        for (auto &p: products_)
        {
            // pair ,  since 
            //std::string ohlc = "/0/public/OHLC";
            //krakenrest_->send_get_req(ohlc, "ohlc");

            std::string orderbook = "/0/public/Depth?pair=" + p;
            krakenrest_->send_get_req(orderbook, "orderbook");

            // pair , since 
            std::string trades = "/0/public/Trades?pair=" + p;
            krakenrest_->send_get_req(trades, "trades");

            // pair , since
            std::string spread = "/0/public/Spread?pair=" + p ;
            krakenrest_->send_get_req(spread, "spread");

            std::this_thread::sleep_for(std::chrono::seconds(15));
        }

	    exittime = ::time(0);
    }
}

}} // dinobot::exchanges::kraken

