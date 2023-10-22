#pragma once
#include "context.h"
#include "timer_handler.h"
#include "trading/md_listener.h"

namespace miye
{
namespace trading
{
namespace ftx
{

struct PerpFtx : public MDListener
{
    int32_t onSnapshotFinished(int32_t cid) override
    {
        logger()->info("onSnapshotFinished cid:{}", cid);
        return 0;
    }
    int32_t onBookChange(int32_t cid) override
    {
        // logger()->info("onBookChange cid:{}", cid);
        return 0;
    }

    int32_t onTrade(uint64_t timestamp, int32_t cid, uint64_t tradeId, Side side, price_t price, quantity_t qty,
                    bool isDone) override
    {
        auto const& symbol = context.symbols[cid];

        logger()->info("onTrade cid:{} symbol:{} price:{} qty:{}", cid, symbol, price, qty);

        // context.risk.setRefPrice(cid, price);
        // context.positions.setMarkPrice(symbol, price);
        ////         maker.updatePrice
        ////         taker.updatePrice
        ////         hedger.updatePrice
        // context.taker.updatePrice(timestamp, cid, symbol, price);
        // context.hedger.updatePrice(timestamp, cid, symbol, price);
        return 0;
    }
    int32_t onTick(int32_t cid) override
    {
        logger()->info("onTick cid:{}", cid);
        return 0;
    }

    void onTimeout() { timerHandler_.onTimeout(); }

    int32_t init(logger::Logger* logger, const std::string configFile);
    logger::Logger* logger() { return logger_; }

    logger::Logger* logger_{nullptr};
    Context context;
    TimerHandler timerHandler_{context};

    bool isStratEnabled{false};
};

} // namespace ftx
} // namespace trading
} // namespace miye
