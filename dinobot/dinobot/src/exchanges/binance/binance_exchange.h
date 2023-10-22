#ifndef _DINOBOT_EXCHANGES_binance_H
#define _DINOBOT_EXCHANGES_binance_H

#include "../../libs/rest/rest_client.h" 
#include "../../libs/configs/configs.h"
#include "../../libs/rings/ring_logger.h"

#include "binance_websocket.h"

namespace dinobot { namespace exchanges { 

class binance_exchange
{
public:
    binance_exchange(dinobot::lib::configs &);
    ~binance_exchange();
    void start();
    void init();
    void set_finish_time(time_t);

private:
    std::unique_ptr<dinobot::exchanges::binance::binance_websocket> binancews_;
    std::unique_ptr<dinobot::lib::rest::rest_client>  binancerest_;

    dinobot::lib::shm::ring_logger * binance_logger_;
    std::thread logger_thread_;

    // finish time that the app will stop ... unless stopped earlier...
    time_t finish_;
};

}} // dinobot::exchanges::binance

#endif 
