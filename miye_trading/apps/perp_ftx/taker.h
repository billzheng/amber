#pragma once

#include "context.h"
#include "libcore/types/types.hpp"
#include "libs/logger/logger.hpp"
#include "market_data/book/order_book.h"
#include "symbol_index.h"
#include "trading/order_manager.h"

namespace miye::trading::ftx
{

struct TakerParam
{
    double bidThresh{NAN};
    double askThresh{NAN};
};

struct Taker
{

    Taker(risk::Risk& risk, position::PortforlioPositions& positions, const std::vector<symbol_t>& symbols,
          OrderManager& om)
        : risk(risk), positions(positions), symbols(symbols), om(om)
    {
    }

    void evaluate()
    {
        if (!isTakeAllowed())
        {
            return;
        }

        if (!validateCondition())
        {
            return;
        }

        const symbol_t symbol;

        // place order
        //        if (buyTakeAllowed && math::greaterEqual(bookSignal_.getValue(), 0.0))
        //        {
        //            auto const side     = Side::BUY;
        //            quantity_t orderQty = takeQty - instPos.getOpenBuy();
        //            price_t orderPrice{NAN};
        //
        //            auto const& risk = context.risk;
        //            if (risk.allowOrder())
        //            {
        //            }
        //        }
    }

    void updatePrice(uint64_t timestamp, int32_t cid, const symbol_t& symbol, price_t price)
    {
        // book is already changed
        // risk/positions are also changed
        // what to do here?
    }

    bool isTakeAllowed() const { return buyTakeAllowed && sellTakeAllowed; }
    bool isBuyAllowed() const { return buyTakeAllowed; }
    bool isSellAllowed() const { return sellTakeAllowed; }

    bool validateCondition()
    {
        //        auto const* instPos = positions.getSymPos(symbol);
        //        if (!instPos)
        //        {
        //            logger()->critical("symbol position object not defined for symbol:{}", symbol);
        //            return false;
        //        }

        if (!watchBook)
        {
            logger()->critical("watch book is not inited, return");
            return false;
        }
        if (!buyTakeAllowed)
        {
            logger()->critical("taker is not inited, return");
            return false;
        }
        return true;
    }

    int init(logger::Logger* logger)
    {
        logger_ = logger;

        return 0;
    }
    logger::Logger* logger() { return logger_; }

  private:
    logger::Logger* logger_{nullptr};
    risk::Risk& risk;
    position::PortforlioPositions& positions;
    const std::vector<symbol_t>& symbols;
    OrderManager& om;

    SymbolIndex watchSymbol{SymbolIndex::BINANCE_FTMUSDT};

    OrderBook* watchBook{nullptr};
    //    Signal& bookSignal_;
    //    Signal& tradeSignal_;

    double bidThresh{NAN};
    double askThresh{NAN};

    bool buyTakeAllowed{false};
    bool sellTakeAllowed{false};

    quantity_t takeQty{0.0};
};

} // namespace miye::trading::ftx
