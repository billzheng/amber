#ifndef _DINOBOT_EXCHANGES_bitmex_H
#define _DINOBOT_EXCHANGES_bitmex_H

#include "../../libs/rest/rest_client.h" 

#include "../../libs/configs/configs.h"
#include "../../libs/rings/ring_logger.h"

#include "bitmex_websocket.h"

namespace dinobot { namespace exchanges { 

class bitmex_exchange
{
public:
    bitmex_exchange(dinobot::lib::configs &);
    ~bitmex_exchange();
    void start();
    void init();
    void set_finish_time(time_t);

private:
    std::unique_ptr<dinobot::exchanges::bitmex::bitmex_websocket> bitmexws_;
    std::unique_ptr<dinobot::lib::rest::rest_client>  bitmexrest_;

    dinobot::lib::shm::ring_logger * bitmex_ws_logger_;
   
    std::thread logger_thread_;

    // finish time that the app will stop ... unless stopped earlier...
    time_t finish_;

    // list of products 
    std::vector<std::string> products_;
};

}} // dinobot::exchanges::bitmex

#endif 
