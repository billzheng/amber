#pragma once
#include "context.h"
#include "libs/logger/logger.hpp"
#include "market_data/book/bbo.h"
#include "position/position.h"
#include "risk/risk.h"
#include "symbol_index.h"
#include "trading/order_manager.h"

namespace miye::trading::ftx
{

enum class SignalSource : uint8_t
{
    BookPressure,
    TradeSignal,
    TradeFlow,
    TradeFlowCrossX, // cross exchange trade flow signal
    BookCorrelation,
    BookCorrelationX, // across exchange book correlation
};

struct Hedger
{
    // explicit Hedger(OrderManager& om, const BBO& bbo, position::SymbolPosition& pos) : om_(om), bbo_(bbo),
    // symPos_(pos)
    Hedger(risk::Risk& risk, position::PortforlioPositions& positions, const std::vector<symbol_t>& symbols,
           OrderManager& om)
        : risk(risk), positions(positions), symbols(symbols), om(om)
    {
    }
    // book
    // position
    void updatePrice(uint64_t timestamp, int32_t cid, const symbol_t& symbol, price_t price) {}

    void updateWatchPrice();
    void updateUnhedgedPosition();
    void hedge();
    bool isHedgeAllowed() const { return isHedgeAllowed_; }

    void cancelFarOrder();
    bool cancelAll()
    {
        auto const& symbol = symbols[to_underlying(hedgeSym)];
        om.cancelAll(symbol);
        return true;
    }

  private:
    // OrderManager& om_;
    // const BBO& bbo_;
    // position::SymbolPosition& symPos_;
    logger::Logger* logger_{nullptr};
    risk::Risk& risk;
    position::PortforlioPositions& positions;
    const std::vector<symbol_t>& symbols;

    OrderManager& om;

    /*
     * TODO: right now we trade on 2 signals so just put them here
     * in the future put them in the alpha_ste or vector?
     */
    //    Signal lastTradeSignal_{};
    //    Signal lastBookSignal_{};

    SymbolIndex hedgeSym{SymbolIndex::FTX_FTM_PERP};

    bool isHedgeAllowed_{false};
};

inline void Hedger::hedge()
{
    if (!isHedgeAllowed())
    {
        cancelAll();
        return;
    }

    // TODO: last position signal is from price or position will help hedger
    // less aggressively?

    // distribute unhedged positions from L3 - further levels.
    // if trade volume is small, just put all unhedged positions out

    //    auto const& symbol = symbols[to_underlying(hedgeSym)];
    //    auto symPos        = *positions.getSymPos(symbol);
    //
    //    auto const netPos      = symPos.getDayNetPosition();
    //    //auto const unHedgedPos = netPos > 0 ? netPos - symPos.getOpenBuy() : (netPos + symPos.getOpenSell());
    //    if (netPos < 0)
    //    {
    //        assert(netPos >= unHedgedPos);
    //    }
    //    else if (netPos > 0)
    //    {
    //        assert(netPos <= unHedgedPos);
    //    }
    //    else if (netPos == 0)
    //    {
    //        cancelAll();
    //    }
}

/*
 * when market moves away from current hedge order over certain steps
 * cancel the order
 */
inline void Hedger::cancelFarOrder() {}

} // namespace miye::trading::ftx
