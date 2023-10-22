#include "bitfinex_websocket.h"


namespace dinobot { namespace exchanges { namespace bitfinex {


bitfinex_websocket::~bitfinex_websocket()
{
    // TODO delete the objects and close ... 
    //stop threads then unsubscribe ..
}



void bitfinex_websocket::unsubscribe()
{
    
}

void bitfinex_websocket::start()
{
    started_ = true;

    thread_.push_back(std::thread(&bitfinex_websocket::init, this));
}

void bitfinex_websocket::parse_json(char *, size_t, uint64_t)
{
}

}}} // dinobot::exchanges::bitfinex

