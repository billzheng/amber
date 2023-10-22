#include <thread>


#include "kraken_websocket.h"


namespace dinobot { namespace exchanges { namespace kraken {


kraken_websocket::~kraken_websocket()
{
    // TODO delete the objects and close ... 
    //stop threads then unsubscribe ..
}


void kraken_websocket::unsubscribe()
{
    

}

void kraken_websocket::send(std::string &test)
{
    //ws_.send(test);
    (void ) test;
}

void kraken_websocket::start()
{
    started_ = true;

    thread_.push_back(std::thread(&kraken_websocket::init, this));
}

void kraken_websocket::parse_json(char *, size_t, uint64_t)
{
}

}}} // dinobot::exchanges::kraken

