#ifndef _DINOBOT_EXCHANGES_bitmex_websocket_H
#define _DINOBOT_EXCHANGES_bitmex_websocket_H

#include "../../libs/websocket/websocket.h"

namespace dinobot { namespace exchanges { namespace bitmex {

class bitmex_websocket : public dinobot::lib::websocket::websocket
{
public:
    bitmex_websocket(uint16_t a, std::string &b, int c, int d, int e) : websocket(a,b,c,d,e) {}
    ~bitmex_websocket();
    void start();
    void unsubscribe();

    void parse_json(char *, size_t, uint64_t);

private:
    bool running_;
};

}}} // dinobot::websockets::bitmex

#endif 
