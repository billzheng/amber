#include "coinbase_websocket.h"


namespace dinobot { namespace exchanges { namespace coinbase {


coinbase_websocket::~coinbase_websocket()
{
    // TODO delete the objects and close ... 
    //stop threads then unsubscribe ..
}



void coinbase_websocket::unsubscribe()
{
    
}

void coinbase_websocket::start()
{
    started_ = true;

    thread_.push_back(std::thread(&coinbase_websocket::init, this));
}

void coinbase_websocket::parse_json(char *, size_t, uint64_t)
{
}

}}} // dinobot::exchanges::coinbase

