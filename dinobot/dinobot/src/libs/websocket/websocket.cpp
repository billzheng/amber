/**
 *  TODO PUSHER integration 
 */
#include <thread>
#include <iostream>
#include "websocket.h"

namespace dinobot { namespace lib { namespace websocket {

websocket::websocket(uint16_t max_con, std::string & ring_fn, int ring_size, int ring_elems, int ring_readers)
    : started_(false)
    , max_subs_per_connection_((!max_con) ? (uint16_t)65535 : max_con )
    , curr_connection_id_(0)
{
    out_ = std::make_unique<lib::shm::ring_writer>(ring_fn, ring_size, ring_elems, ring_readers);
}


websocket::~websocket() 
{
    for(auto& thread : thread_)
        thread.join();
}

void websocket::parse_json(char * , size_t , uint64_t )
{
    
}

uint32_t websocket::add_connection(std::string &uri)
{
    if (started_)
        throw std::runtime_error("websocket::add_connections: slready started, can't add connections, exiting");

    uri_[curr_connection_id_] = uri;
    return (curr_connection_id_++); // return then increment
}

bool websocket::add_stream(uint32_t id, const std::string &s)
{
    if (started_ || (id >= curr_connection_id_))
    {
        std::cerr << "websocket::add_stream: already started or id not found" << std::endl;
        return false;
    }

    streams_[id].push_back(s);

    return true;
}

bool websocket::add_subscription(uint32_t id, const std::string &s)
{
    if (started_ || (id >= curr_connection_id_))
    {
        std::cerr << "websocket::add_subscription: already started or id not found" << std::endl;
        return false;
    }

    // count current list if greater than max_co max_subs_per_connection_ then fail 
    auto subs = subscriptions_[id];
    if (subs.size() >= max_subs_per_connection_)
    {
        std::cerr << "websocket::add_subscription: max connections per socket hit" << std::endl;
        return false;
    }

    subscriptions_[id].push_back(s);

    return true;
}

void websocket::init()
{
    on_connection();

    on_message();

    // loop through connections 
    for (auto &u : uri_)
    {
        auto id = u.first;
        auto uri = u.second;
        
        if (streams_[id].size() > 0)
        {
            for (auto &s: streams_[id])
            {
                ws_.connect(s, nullptr); // TODO maybe a better way than this 
            }
        }
        else 
        {
            ws_.connect(uri, (void *) (uint64_t) id);
        }
    }

    // set the dissconnect handler TODO // fix what happens if we get dissconnected
    on_disconnect();

    std::cout << "starting to run the websocket server" << std::endl;
    ws_.run();
}

void websocket::on_connection()
{
    ws_.onConnection([&subscriptions_ = subscriptions_, &uri_ = uri_](uWS::WebSocket<uWS::CLIENT> *ws, uWS::HttpRequest req) {
        (void)req;
        auto conn_type = (uint32_t) (uint64_t) ws->getUserData();
        
        for (auto &subs: subscriptions_[conn_type])
        {
            std::cout << "subscribing to " << uri_[conn_type] << " " << subs << std::endl;
            ws->send(subs.c_str(), subs.length(), uWS::OpCode::TEXT);
        }
    });
}

void websocket::on_message()
{
    ws_.onMessage([&out_ = out_, this ](uWS::WebSocket<uWS::CLIENT> *ws, char *message, size_t length, uWS::OpCode opCode) {
        (void) ws ;
        (void) opCode;

        //auto conn_type = (uint32_t) (uint64_t) ws->getUserData();
        std::chrono::system_clock::time_point received_tp = std::chrono::system_clock::now();
        //std::cout << received_tp.time_since_epoch().count() << " " << conn_type << " " <<  length << " " << std::string(message, length)  << std::endl;
        
        out_->write(message, length, received_tp.time_since_epoch().count(), 0);

        this->parse_json(message, length, received_tp.time_since_epoch().count());

        //TODO RE ADD // const sajson::document& document = sajson::parse(sajson::dynamic_allocation(), sajson::mutable_string_view(length, message));
        // TODOexchange::coinbase::websocket::parse_json(document);
    });
}

void websocket::on_disconnect()
{
    ws_.onDisconnection([this](uWS::WebSocket<uWS::CLIENT> *ws, int code, char *message, size_t length) {
        std::cout << "Client got disconnected with data: " << ws->getUserData() << ", code: " << code << ", message: <" << std::string(message, length) << ">" << std::endl;
        while(true)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            // lets try to init
            init();
        }
    });
}


}}}// dinobot::lib
