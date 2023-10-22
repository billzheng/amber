#ifndef _DINOBOT_EXCHANGES_bitstamp_H
#define _DINOBOT_EXCHANGES_bitstamp_H

#include "../../libs/rest/rest_client.h" 

#include "../../libs/configs/configs.h"
#include "../../libs/websocket/websocket.h"
#include "../../libs/rings/ring_logger.h"

#include "bitstamp_websocket.h"

namespace dinobot { namespace exchanges { 

class bitstamp_exchange
{
public:
    bitstamp_exchange(dinobot::lib::configs &);
    ~bitstamp_exchange();
    void start();
    void init();
    void set_finish_time(time_t);

private:
    std::unique_ptr<dinobot::exchanges::bitstamp::bitstamp_websocket> bitstampws_;
    std::unique_ptr<dinobot::lib::rest::rest_client>  bitstamprest_;

    //dinobot::lib::websocket::websocket * bitstampws_;
    //dinobot::lib::rest::rest_client * bitstamprest_;
    dinobot::lib::shm::ring_logger * bitstamp_ws_logger_;
    //dinobot::lib::shm::ring_logger * bitstamp_rest_logger_;
    std::thread logger_thread_;

    // finish time that the app will stop ... unless stopped earlier...
    time_t finish_;

    // list of products 
    std::vector<std::string> products_;
};

}} // dinobot::exchanges::bitstamp

#endif 
