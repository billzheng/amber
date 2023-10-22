#ifndef _DINOBOT_EXCHANGES_H
#define _DINOBOT_EXCHANGES_H

#include "../libs/configs/configs.h"
#include "../libs/websocket/websocket.h"
#include "../libs/rest/rest_client.h" 

namespace dinobot { namespace exchanges { 

class exchange_base
{
public:
    exchange_base(dinobot::lib::configs &);
    ~exchange_base();
    void set_finish_time(time_t);

    virtual void start() = 0;
    virtual void init() = 0;

protected:
    //std::unique_ptr<dinobot::lib::websocket::websocket> websocket_;
    //std::unique_ptr<dinobot::lib::rest::rest_client>  rest_;

    // finish time that the app will stop ... unless stopped earlier...
    time_t finish_;

    // list of available products we can use for this exchange
    std::vector<std::string> products_;

    std::string uri_; 
    std::string path_; 
    int ring_mtu_; 
    int ring_num_readers_;
    int ring_elements_;
    std::string ring_logger_file_;
};

}} // dinobot::exchange

#endif 
