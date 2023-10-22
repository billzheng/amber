#ifndef _DINOBOT_EXCHANGES_kraken_H
#define _DINOBOT_EXCHANGES_kraken_H

#include "../../libs/rest/rest_client.h" 

#include "../../libs/configs/configs.h"
#include "../../libs/rings/ring_logger.h"

#include "kraken_websocket.h"

namespace dinobot { namespace exchanges { 

class kraken_exchange
{
public:
    kraken_exchange(dinobot::lib::configs &);
    ~kraken_exchange();
    void start();
    void init();
    void set_finish_time(time_t);

private:
    std::unique_ptr<dinobot::lib::rest::rest_client>  krakenrest_;

    dinobot::lib::shm::ring_logger * kraken_ws_logger_;
   
    std::thread logger_thread_;

    // finish time that the app will stop ... unless stopped earlier...
    time_t finish_;

    // list of products 
    std::vector<std::string> products_;
    std::string products_str_;
};

}} // dinobot::exchanges::kraken

#endif 
