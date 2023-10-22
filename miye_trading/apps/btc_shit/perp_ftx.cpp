
#include "perp_ftx.h"

namespace miye::trading ::ftx
{

int32_t PerpFtx::init(logger::Logger* logger, const std::string configFile)
{
    logger_ = logger;

    context.init(logger, configFile);
    timerHandler_.init(logger_);
    timerHandler_.startTimers(time::NanoClock::now());
    return 0;
}

} // namespace miye::trading::ftx
