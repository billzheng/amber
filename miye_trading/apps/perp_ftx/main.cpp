#include "application.h"
#include "config_loader.h"
#include "market_data/market_main.h"
#include "perp_ftx.h"

#include <chrono>
#include <iostream>
#include <thread>

int main(int argc, char** argv)
{
    using namespace miye;

    trading::ftx::PerpFtxApp app;
    app.run(argc, argv);
}
