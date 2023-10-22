#include <thread>


#include "deribit_websocket.h"


namespace dinobot { namespace exchanges { namespace deribit {


deribit_websocket::~deribit_websocket()
{
    // TODO delete the objects and close ... 
    //stop threads then unsubscribe ..
}


void deribit_websocket::unsubscribe()
{
    

}

void deribit_websocket::send(std::string &test)
{
    //ws_.send(test);
    (void ) test;
}

void deribit_websocket::start()
{
    started_ = true;

    thread_.push_back(std::thread(&deribit_websocket::init, this));
}

void deribit_websocket::parse_json(char *, size_t, uint64_t)
{
}

}}} // dinobot::exchanges::deribit

