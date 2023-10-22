#ifndef _DINOBOT_EXCHANGES_deribit_H
#define _DINOBOT_EXCHANGES_deribit_H

#include "../../libs/rest/rest_client.h" 

#include "../../libs/configs/configs.h"
#include "../../libs/rings/ring_logger.h"

#include "deribit_websocket.h"

namespace dinobot { namespace exchanges { 

class deribit_exchange
{
public:
    deribit_exchange(dinobot::lib::configs &);
    ~deribit_exchange();
    void start();
    void init();
    void set_finish_time(time_t);

private:
    std::unique_ptr<dinobot::exchanges::deribit::deribit_websocket> deribitws_;
    std::unique_ptr<dinobot::lib::rest::rest_client>  deribitrest_;

    dinobot::lib::shm::ring_logger * deribit_ws_logger_;
   
    std::thread logger_thread_;

    // finish time that the app will stop ... unless stopped earlier...
    time_t finish_;

    // list of products 
    std::vector<std::string> products_;
};

}} // dinobot::exchanges::deribit

#endif 
