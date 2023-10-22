#include <thread>


#include "bitstamp_websocket.h"


namespace dinobot { namespace exchanges { namespace bitstamp {


bitstamp_websocket::~bitstamp_websocket()
{
    // TODO delete the objects and close ... 
    //stop threads then unsubscribe ..
}


void bitstamp_websocket::unsubscribe()
{
    
}

void bitstamp_websocket::start()
{
    started_ = true;

    thread_.push_back(std::thread(&bitstamp_websocket::init, this));
}

void bitstamp_websocket::parse_json(char *, size_t, uint64_t)
{

}

}}} // dinobot::exchanges::bitstamp

