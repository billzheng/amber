#ifndef _DINOBOT_EXCHANGES_bitfinex_websocket_H
#define _DINOBOT_EXCHANGES_bitfinex_websocket_H

#include "../../libs/websocket/websocket.h"

namespace dinobot { namespace exchanges { namespace bitfinex {

class bitfinex_websocket : public dinobot::lib::websocket::websocket
{
public:
    bitfinex_websocket(uint16_t a, std::string &b, int c, int d, int e) : websocket(a,b,c,d,e) {}
    ~bitfinex_websocket();
    void start();
    void unsubscribe();

    void parse_json(char *, size_t, uint64_t);
};

}}} // dinobot::websockets::bitfinex

#endif 
