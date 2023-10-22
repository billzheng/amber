#include <thread>


#include "bitfinex_exchange.h"

#include "../../libs/rings/ring_logger.h"

namespace dinobot { namespace exchanges {

bitfinex_exchange::bitfinex_exchange(dinobot::lib::configs &c)
    : bitfinexws_(nullptr)
    , bitfinexrest_(nullptr)
    , bitfinex_ws_logger_(nullptr)
    , products_()
{
    std::string uri = c.get_config<std::string>("exchange_ws", "uri");
    std::string path = c.get_config<std::string>("ring_writer_md", "path");
    bitfinexws_ = std::make_unique<dinobot::exchanges::bitfinex::bitfinex_websocket>(0,
                                                                      path,
                                                                      c.get_config<int>("ring_writer_md", "mtu"),
                                                                      c.get_config<int>("ring_writer_md", "elements"),
                                                                      c.get_config<int>("ring_writer_md", "num_readers") );
    uint32_t c_id = bitfinexws_->add_connection(uri);

    for (auto &n: c.get_startup_trade_data<std::vector<std::string>>("bitfinex", "symbol_list"))
    {
        products_.push_back(n);
        // lets start the websocket stuff
        std::string sub = "{\"event\":\"subscribe\",\"channel\":\"book\",\"pair\":\"" + n + "\",\"prec\":\"R0\"}";
        bitfinexws_->add_subscription(c_id, sub);
        std::string trd = "{\"event\":\"subscribe\",\"channel\":\"trades\",\"pair\":\"" + n + "\"}";
        bitfinexws_->add_subscription(c_id, trd);
    }

    for (auto &n: c.get_startup_trade_data<std::vector<std::string>>("bitfinex", "currencies"))
    {
        currencies_.push_back(n);
    }

    bitfinexws_->start();
    

    // init everything
    //init();

    // 2 second sleep to make sure we have subscribed
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    bitfinex_ws_logger_ = new dinobot::lib::shm::ring_logger(c.get_config<std::string>("ring_writer_md", "path"), 
                                                         c.get_config<std::string>("logs", "path_ws"), 0/*add this to config somehow TODO*/);
    
    logger_thread_ = bitfinex_ws_logger_->start_thread();

    // rest clients 
    bitfinexrest_ = std::make_unique<dinobot::lib::rest::rest_client>(c);
}

bitfinex_exchange::~bitfinex_exchange()
{
    // TODO delete the objects and close ... 
}

void bitfinex_exchange::set_finish_time(time_t midnight)
{
    finish_ = midnight;
}

void bitfinex_exchange::init()
{
    
}

/* 
 * Start the market data reading ... 
 */
void bitfinex_exchange::start()
{
	time_t exittime = time(0);
    time_t booktime = time(0);

    std::string symbols = "/v1/symbols";
    bitfinexrest_->send_get_req(symbols, "symbols");

    std::string symbols_details = "/v1/symbols_details";
    bitfinexrest_->send_get_req(symbols_details, "symbols_details");

    while (difftime(finish_, exittime) > 0)
    {
	    exittime = time(0);
	    std::this_thread::sleep_for(std::chrono::milliseconds(500)); 

        if (difftime(booktime, exittime) < 0)
        {
            int count = 0;
            for (auto &p: products_)
            {
                count++;
                if (count >= 25) // 30 /second are allowed of this request
                {
	                std::this_thread::sleep_for(std::chrono::seconds(60)); 
                    count = 0;
                }
                std::cout << "requesting order book: " << p << std::endl;
                std::string temp = p + "_orderbook";
                std::string orderbook = "/v1/book/" + p + "?limit_bids=-1&limit_asks=-1";
                bitfinexrest_->send_get_req(orderbook, temp);
            }

            count = 0;
            for (auto &c: currencies_)
            {
                count++;
                // 10 req / minute rate
                if (count >=9)
                {
	                std::this_thread::sleep_for(std::chrono::seconds(60)); 
                    count = 0;
                }
                std::cout << "requesting funding book: " << c << std::endl;
                std::string fundingbook = "/v1/lendbook/" + c + "?limit_bids=1000&limit_asks=1000"; // 1000 seems ok here since 0 does nothing at this endpoint
                std::string temp = c + "_fundingbook";
                bitfinexrest_->send_get_req(fundingbook, temp);
            }

            count = 0;
            for (auto &c: currencies_)
            {
                count++;
                // 10 req / minute rate
                if (count >=9)
                {
	                std::this_thread::sleep_for(std::chrono::seconds(60)); 
                    count = 1;
                }
                std::cout << "requesting lending: " << c << std::endl;
                std::string lending = "/v1/lends/" + c + "?limit_lends=1000";
                std::string temp = c + "_lending";
                bitfinexrest_->send_get_req(lending, temp);
            }

            booktime = time(0) + 600; // 5 minutes in the future
        }

	    // here we are checking if we have 
        //
	    if (difftime(finish_, exittime) < 0)
	        break;

    }
}

}} // dinobot::exchanges::bitfinex

