//#include "libcore/web_socket/web_socket.h"
//#include "libs/libwebsockets/include/libwebsockets.h"

#include <chrono>
#include <iostream>
#include <stdint.h>

int ws_depth_onData(Json::Value &json_result)
{
    int new_updateId = json_result["u"].asUInt64();
    std::cout << "updateId:" << new_updateId << std::endl;
    // std::cout << " get data" << std::endl;
    return 0;
}

int main(int argc, char *argv[])
{
    auto const lwsContext =
        miye::ws::makeWsContext("example-name", miye::ws::WsEventCB);

    miye::ws::connectEndpoint(lwsContext,
                              "fstream3.binance.com",
                              443,
                              "example-name",
                              ws_depth_onData,
                              "/ws/btcusdt@depth5@0ms");

    miye::ws::WSEventLoop(lwsContext);

    return 0;
}
