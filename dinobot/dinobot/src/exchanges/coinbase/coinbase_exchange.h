#ifndef _DINOBOT_EXCHANGES_coinbase_H
#define _DINOBOT_EXCHANGES_coinbase_H

#include "../../libs/rest/rest_client.h" 

#include "../../libs/configs/configs.h"
#include "../../libs/rings/ring_logger.h"

#include "coinbase_websocket.h"

namespace dinobot { namespace exchanges { 

class coinbase_exchange
{
public:
    coinbase_exchange(dinobot::lib::configs &);
    ~coinbase_exchange();
    void start();
    void init();
    void set_finish_time(time_t);

private:
    std::unique_ptr<dinobot::exchanges::coinbase::coinbase_websocket> coinbasews_;
    std::unique_ptr<dinobot::lib::rest::rest_client>  coinbaserest_;

    dinobot::lib::shm::ring_logger * coinbase_ws_logger_;
    std::thread logger_thread_;

    // finish time that the app will stop ... unless stopped earlier...
    time_t finish_;

    // list of products 
    std::vector<std::string> products_;
};

}} // dinobot::exchanges::coinbase

#endif 
