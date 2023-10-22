#ifndef _DINOBOT_EXCHANGES_bitstamp_websocket_H
#define _DINOBOT_EXCHANGES_bitstamp_websocket_H

#include "../../libs/websocket/websocket.h"

namespace dinobot { namespace exchanges { namespace bitstamp {

class bitstamp_websocket : public dinobot::lib::websocket::websocket
{
public:
    bitstamp_websocket(uint16_t a, std::string &b, int c, int d, int e) : websocket(a,b,c,d,e) {}
    ~bitstamp_websocket();
    void start();
    void unsubscribe();

    void parse_json(char *, size_t, uint64_t);

};

}}} // dinobot::websockets::bitstamp

#endif 
