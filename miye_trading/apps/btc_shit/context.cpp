#include "context.h"
#include "config_loader.h"

namespace miye::trading::ftx
{

int32_t Context::init(logger::Logger* logger, std::string config)
{
    logger_ = logger;
    logger->info("loading instrument from file:{}", config);

    stratParam.instruments = ConfigLoader::getInstrumentInfo(config);

    std::cout << "symbol size:" << stratParam.instruments.size() << std::endl;

    for (auto const& i : stratParam.instruments)
    {
        std::cout << i << std::endl;
        logger->info("instrument:{}", i);
    }

    //    positions.init(logger);
    //    risk.init(logger);
    //    taker.init(logger);
    return 0;
}

} // namespace miye::trading::ftx
