#ifndef _DINOBOT_WEBSOCKET_H
#define _DINOBOT_WEBSOCKET_H

#include <map>
#include <vector>
#include <thread>

#include "../rings/ring_writer.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter" // the uWS code doe not compile with our settings because of unused parameters
#ifdef __APPLE__
#include "../../../build/external/uWebSockets/include/uWS/uWS.h"  // no idea why we need this :( rather than the normal place. 
#else
#include "uWS/uWS.h"
#endif
#pragma GCC diagnostic pop


namespace dinobot { namespace lib { namespace websocket {

class websocket
{
public:
    websocket(uint16_t, std::string &, int, int, int);
    ~websocket();
    
    uint32_t add_connection(std::string &);
    bool add_stream(uint32_t, const std::string &);
    bool add_subscription(uint32_t, const std::string &);

    virtual void start() = 0;
    virtual void unsubscribe() = 0;
    virtual void parse_json(char *, size_t, uint64_t) = 0;

private:
    void on_connection();
    void on_message();
    void on_disconnect();

protected:
    void init();

protected:
    bool started_;
    std::vector<std::thread> thread_;
    uWS::Hub ws_;

private:
    uint16_t max_subs_per_connection_;
    uint32_t curr_connection_id_;
    
    std::unique_ptr<dinobot::lib::shm::ring_writer> out_; 

    // each connection has 1 url (that may or may not be unique)
    std::map<uint32_t, std::string> uri_;

    // list of streams or subscriptions, some exchanges fail if we try to load 
    // up the subscription with all the exchanges in one message, so it's best
    // store them here and iterate through on connection (assuming we don't hit
    // rate limits etc)
    std::map<uint32_t, std::vector<std::string>> streams_;
    std::map<uint32_t, std::vector<std::string>> subscriptions_;
};

}}}  // dinobot :: lib ::  websocket

#endif
