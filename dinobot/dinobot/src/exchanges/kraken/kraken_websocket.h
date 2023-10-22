#ifndef _DINOBOT_EXCHANGES_kraken_websocket_H
#define _DINOBOT_EXCHANGES_kraken_websocket_H

#include "../../libs/websocket/websocket.h"

namespace dinobot { namespace exchanges { namespace  kraken {

class  kraken_websocket : public dinobot::lib::websocket::websocket
{
public:
    kraken_websocket(uint16_t a, std::string &b, int c, int d, int e) : websocket(a,b,c,d,e) {}
    ~kraken_websocket();
    void start();
    void unsubscribe();

    void send(std::string &);

    void parse_json(char *, size_t, uint64_t);
};

}}} // dinobot::websockets:: kraken

#endif 
