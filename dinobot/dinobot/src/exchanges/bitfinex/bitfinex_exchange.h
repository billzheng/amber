#ifndef _DINOBOT_EXCHANGES_bitfinex_H
#define _DINOBOT_EXCHANGES_bitfinex_H

#include "../../libs/rest/rest_client.h" 

#include "../../libs/configs/configs.h"
#include "../../libs/rings/ring_logger.h"

#include "bitfinex_websocket.h"

namespace dinobot { namespace exchanges { 

class bitfinex_exchange
{
public:
    bitfinex_exchange(dinobot::lib::configs &);
    ~bitfinex_exchange();
    void start();
    void init();
    void set_finish_time(time_t);

private:
    std::unique_ptr<dinobot::exchanges::bitfinex::bitfinex_websocket> bitfinexws_;
    std::unique_ptr<dinobot::lib::rest::rest_client>  bitfinexrest_;
    
    dinobot::lib::shm::ring_logger * bitfinex_ws_logger_;
    std::thread logger_thread_;

    // finish time that the app will stop ... unless stopped earlier...
    time_t finish_;

    // list of products 
    std::vector<std::string> products_;
    std::vector<std::string> currencies_;
};

}} // dinobot::exchanges::bitfinex

#endif 
