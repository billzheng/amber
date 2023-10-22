#ifndef _DINOBOT_EXCHANGES_coinbase_websocket_H
#define _DINOBOT_EXCHANGES_coinbase_websocket_H

#include "../../libs/websocket/websocket.h"

namespace dinobot { namespace exchanges { namespace coinbase {

class coinbase_websocket : public dinobot::lib::websocket::websocket
{
public:
    coinbase_websocket(uint16_t a, std::string &b, int c, int d, int e) : websocket(a,b,c,d,e) {}
    ~coinbase_websocket();
    void start();
    void unsubscribe();

    void parse_json(char *, size_t, uint64_t);
};

}}} // dinobot::websockets::coinbase

#endif 
