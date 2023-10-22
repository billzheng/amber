#include "binance_websocket.h"


namespace dinobot { namespace exchanges { namespace binance {


binance_websocket::~binance_websocket()
{
    // TODO delete the objects and close ... 
    //stop threads then unsubscribe ..
}


void binance_websocket::unsubscribe()
{
    
}

void binance_websocket::start()
{
    started_ = true;

    thread_.push_back(std::thread(&binance_websocket::init, this));
}

void binance_websocket::parse_json(char *, size_t, uint64_t)
{
}

}}} // dinobot::exchanges::binance

