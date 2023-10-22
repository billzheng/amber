#pragma once
#include "hash_combine.h"
#include "libcore/math/math_utils.hpp"
#include "libcore/types/types.hpp"
#include "libcore/utils/nano_time.h"
#include "libcore/utils/rolling_queue.h"

namespace miye::risk
{

static const constexpr time::NanoDuration TimeoutWindow =
    std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(900));

enum class StateEnum : uint8_t
{
    None = 0,
    MaxServerOrderRate,
    MaxSymbolOrderRate,
    TooManyInternalRejects,
    TooManyExternalRejects,
    SymbolBlackListedReject,
    GlobalBlackListedReject,
    InvalidAcountId,
    InvalidSymbol,
    FillPriceMisMatch,
    FillSizeExceedOrderSize,
    FillSideMisMatch,
    FillPriceOutOfLimitUpLimitDownRange,
    LimitOrderWorseFillPrice,
    MktRejectInvalidSymbol,
    MktRejectOddLotSize,
    MktRejectPrice,
    MktRejectPricePrecision,
    MktRejectInvalidMinQty,
    MktRejectInvalidLotSize,
    NoEnoughLocate
};

inline const char* toString(StateEnum state)
{
    switch (state)
    {
    case StateEnum::MaxServerOrderRate:
        return "MaxServerOrderRate";
    case StateEnum::MaxSymbolOrderRate:
        return "MaxSymbolOrderRate";
    case StateEnum::TooManyInternalRejects:
        return "TooManyInternalRejects";
    case StateEnum::TooManyExternalRejects:
        return "TooManyExternalRejects";
    case StateEnum::SymbolBlackListedReject:
        return "SymbolBlackListedReject";
    case StateEnum::GlobalBlackListedReject:
        return "GlobalBlackListedReject";
    case StateEnum::InvalidAcountId:
        return "InvalidAcountId";
    case StateEnum::InvalidSymbol:
        return "InvalidSymbol";
    case StateEnum::FillPriceMisMatch:
        return "FillPriceMisMatch";
    case StateEnum::FillSizeExceedOrderSize:
        return "FillSizeExceedOrderSize";
    case StateEnum::FillSideMisMatch:
        return "FillSideMisMatch";
    case StateEnum::FillPriceOutOfLimitUpLimitDownRange:
        return "FillPriceOutOfLimitUpLimitDownRange";
    case StateEnum::LimitOrderWorseFillPrice:
        return "LimitOrderWorseFillPrice";
    case StateEnum::MktRejectInvalidSymbol:
        return "MktRejectInvalidSymbol";
    case StateEnum::MktRejectOddLotSize:
        return "MktRejectOddLotSize";
    case StateEnum::MktRejectPrice:
        return "MktRejectPrice";
    case StateEnum::MktRejectPricePrecision:
        return "MktRejectPricePrecision";
    case StateEnum::MktRejectInvalidMinQty:
        return "MktRejectInvalidMinQty";
    case StateEnum::MktRejectInvalidLotSize:
        return "MktRejectInvalidLotSize";
    case StateEnum::NoEnoughLocate:
        return "NoEnoughLocate";
    }

    return "None";
}

struct RiskState
{
    time::NanoTime timeout{};
    StateEnum timeoutState{StateEnum::None};
    StateEnum haltState{StateEnum::None};
    void resetTimeoutState() { timeoutState = StateEnum::None; }
    void setTimeout(time::NanoTime timestamp, StateEnum value)
    {
        timeout      = timestamp;
        timeoutState = value;
    }
    bool isGreen() const { return timeoutState == StateEnum::None && haltState == StateEnum::None; }
    bool isHalted() const { return haltState != StateEnum::None; }

    void setHalt(StateEnum value) { haltState = value; }
    StateEnum getTimeoutState() const { return timeoutState; }
    time::NanoTime gettimeout() const { return timeout; }

    StateEnum getState(time::NanoTime timestamp) const
    {
        if (haltState != StateEnum::None)
        {
            return haltState;
        }
        else if (timestamp <= timeout)
        {
            return timeoutState;
        }
        return StateEnum::None;
    }
};

struct SymbolRisk
{
    RiskState state{};
    price_t refPrice{NAN};
    price_t maxPriceDeviation{NAN};
    price_t limitDown{NAN};
    price_t limitUp{NAN};

    int32_t repeatedOrderRate{};
    size_t repeatedOrderHashValue{0};
    quantity_t minOrderSize{0.0};
    int32_t currentOrderRate{0};
    int32_t currentRejectRate{0};
    quantity_t maxOpen{0.0};
    quantity_t maxOrderSize{0.0};
    quantity_t minmax{0.0};
    price_t minmaxValue{0.0};
    price_t maxOrderValue{0.0};
    price_t priceDeviation{0.0};
    int32_t maxOrderRate{};
    int32_t maxRepeatedOrderRate{};

    time::NanoTime lastResetTimestamp{};

    int32_t init(price_t priceDeviation, price_t limitDown, price_t limitUp)
    {
        this->priceDeviation = priceDeviation;
        this->limitDown      = limitDown;
        this->limitUp        = limitUp;
        return 0;
    }

    bool isHalted() const { return state.isHalted(); }
    void setRefPrice(price_t price)
    {
        this->refPrice = refPrice;

        this->maxPriceDeviation = priceDeviation * price;
    }
    void resetCounters(time::NanoTime timestamp)
    {
        currentOrderRate       = 0;
        currentRejectRate      = 0;
        repeatedOrderHashValue = 0;
        repeatedOrderRate      = 0;
        state.resetTimeoutState();
        lastResetTimestamp = timestamp;
    }
    bool checkBasics(logger::Logger* logger, const symbol_t& symbol, const order_t& order) const
    {
        if (std::isnan(order.qty) || std::isinf(order.qty) || math::greaterThan(order.qty, maxOrderSize) ||
            math::floatEqual(order.qty, 0.0))
        {
            logger->critical("invalid order qty {}", order);
            return false;
        }

        if (std::isnan(order.price) || std::isinf(order.price))
        {
            logger->critical("invalid order price {}", order);

            return false;
        }
        if (order.getSide() != Side::BUY || order.getSide() != Side::SELL)
        {
            logger->critical("invalid order side {}", order);
            return false;
        }

        return false;
    }

    bool hasAvailableSymOrderRate() const { return currentOrderRate < maxOrderRate; }
    bool checkOrderRate(logger::Logger* logger, const symbol_t& symbol)
    {
        if (++currentOrderRate > maxOrderRate)
        {
            logger->error("symbolRisk checkOrderRate symbol:{} maxOrderRate:{}", symbol, maxOrderRate);
            return false;
        }
        return true;
    }
    bool checkRepeatedOrderRate(logger::Logger* logger, const symbol_t& symbol, const order_t& order)
    {
        std::size_t hashValue{};
        hash_combine(
            hashValue, (uint32_t)order.getSide(), order.getEntrySize(), order.getPrice(), order.getOrderType());
        if (repeatedOrderHashValue != hashValue || time::NanoClock::now() - lastResetTimestamp > TimeoutWindow)
        {
            repeatedOrderHashValue = hashValue;
            repeatedOrderRate      = 1;
        }
        else
        {
            repeatedOrderRate++;
        }

        if (repeatedOrderRate > maxOrderRate)
        {
            logger->error(
                "symbolRisk checkRepeatedOrderRate maxRepeatedOrderRate:{} order:{}", maxRepeatedOrderRate, order);
            return false;
        }
        return true;
    }

    bool isPosReducing(logger::Logger* logger, const symbol_t& symbol, const order_t& order,
                       const position::SymbolPosition& pos)
    {
        auto const orderQty = order.getQty();
        if (order.getSide() == Side::BUY)
        {
            return std::abs(pos.getNetPosition() + orderQty) < std::abs(pos.getNetPosition());
        }
        else if (order.getSide() == Side::SELL)
        {
            return std::abs(pos.getNetPosition() - orderQty) < std::abs(pos.getNetPosition());
        }
        return false;
    }
    bool isMinMaxReducing(logger::Logger* logger, const symbol_t& symbol, const order_t& order,
                          const position::SymbolPosition& pos)
    {

        if (order.getSide() == Side::BUY)
        {
            auto const currentMinMax = pos.getNetPosition() + pos.getOpenBuy() + order.getEntrySize();
            return std::abs(currentMinMax) < std::abs(pos.getNetPosition() + pos.getOpenBuy());
        }
        else if (order.getSide() == Side::SELL)
        {
            auto const currentMinMax = pos.getNetPosition() - pos.getOpenSell() - order.getEntrySize();
            return std::abs(currentMinMax) < std::abs(pos.getNetPosition() - pos.getOpenSell());
        }
        return false;
    }

    bool checkSide(logger::Logger* logger, const order_t& order) const
    {
        if (order.getSide() != Side::BUY && order.getSide() != Side::BUY)
        {
            logger->critical("symbolRisk checkSide invalid order side {}", order);
            return false;
        }
        return true;
    }
    bool checkMaxOpen(logger::Logger* logger, const order_t& order, const position::SymbolPosition& pos) const
    {
        auto const openSize = order.getSide() == Side::BUY ? pos.getOpenBuy() : pos.getOpenSell();
        if (openSize + order.getEntrySize() > this->maxOpen)
        {
            logger->critical(
                "symbolRisk checkMaxOpen sum of order entrySize:{} and openSize:{} greater than maxOpen:{}",
                order.getEntrySize(),
                openSize,
                maxOpen);
            return false;
        }
        return true;
    }

    bool checkOrderValue(logger::Logger* logger, const symbol_t& symbol, const order_t& order)
    {
        if (std::isnan(order.getPrice()) || std::isinf(order.getPrice()))
        {
            logger->critical("symbolRisk checkOrderValue invalid order price:{} order:{}", order.getPrice(), order);
            return false;
        }
        auto const orderValue = order.getSize() * order.getPrice();
        if (math::greaterThan(orderValue, maxOrderValue))
        {
            return false;
        }
        return true;
    }

    bool checkMinMax(logger::Logger* logger, const symbol_t& symbol, const order_t& order,
                     const position::SymbolPosition& pos)
    {
        if (order.getSide() == Side::BUY)
        {
            auto const currentMinMax = pos.getNetPosition() + pos.getOpenBuy() + order.getSize();
            // pass non risk increasing orders, this is necessary as we may be already over lmit without the new order
            if (std::abs(currentMinMax) <= std::abs(pos.getNetPosition() + pos.getOpenBuy()))
            {
                return true;
            }
            if (std::abs(currentMinMax) > this->minmax)
            {
                logger->critical("symbolRisk checkMinMax buy failed order {}", order);
                return false;
            }
        }
        else
        {
            auto const currentMinMax = pos.getNetPosition() - pos.getOpenSell() - order.getSize();
            if (std::abs(currentMinMax) <= std::abs(pos.getNetPosition() - pos.getOpenSell()))
            {
                return true;
            }
            if (std::abs(currentMinMax) > this->minmax)
            {
                logger->critical("symbolRisk checkMinMax sell failed order {}", order);
                return false;
            }
        }
        return true;
    }
    bool checkMinMaxValue(logger::Logger* logger, const symbol_t& symbol, const order_t& order,
                          const position::SymbolPosition& pos)
    {
        if (std::isnan(order.getPrice()) || std::isinf(order.getPrice()) || std::isnan(order.getQty()) ||
            std::isinf(order.getQty()) || math::floatEqual(order.getQty(), 0.0))
        {
            logger->critical("symbolRisk checkMinMaxValue invalid order price or qty, order {}", order);
            return false;
        }

        if (order.getSide() == Side::BUY)
        {
            auto const currentMinMaxValue =
                pos.getNetValue() + pos.getOpenBuyValue() + order.getSize() * order.getPrice();
            // pass non risk increasing orders, this is necessary as we may be already over lmit without the new order
            if (std::abs(currentMinMaxValue) <= std::abs(pos.getNetValue() + pos.getOpenBuyValue()))
            {
                return true;
            }
            if (std::abs(currentMinMaxValue) > this->minmax)
            {
                logger->critical("symbolRisk checkMinMaxValue buy failed order {}", order);
                return false;
            }
        }
        else
        {
            auto const currentMinMaxValue =
                pos.getNetValue() - pos.getOpenBuyValue() - order.getSize() * order.getPrice();
            if (std::abs(currentMinMaxValue) <= std::abs(pos.getNetValue() - pos.getOpenSellValue()))
            {
                return false;
            }
            if (std::abs(currentMinMaxValue) > this->minmax)
            {
                logger->critical("symbolRisk checkMinMaxValue sell failed order {}", order);
                return false;
            }
        }
        return true;
    }

    StateEnum reconcileFill(logger::Logger* logger, const symbol_t& symbol, const order_t& order, Side fillSide,
                            price_t fillPrice, quantity_t fillQty) const
    {
        if (order.getSide() != fillSide)
        {
            logger->critical("symbolRisk reconcileFill failed, fill side:{} order:{}", toString(fillSide), order);
            return StateEnum::FillSideMisMatch;
        }
        if (math::greaterThan(order.getEntrySize(), fillQty))
        {
            return StateEnum::FillSizeExceedOrderSize;
        }

        // not check fill price now. I heard some exchange give different fill price even for limit order

        return StateEnum::None;
    }
};

struct GlobalRisk
{
    RiskState state{};
    uint16_t maxOrderRate{};
    uint16_t maxRejectRate{};
    uint16_t maxRepeatedOrderRate{};
    quantity_t minmax{};
    double minmaxValue{};
    double buyingPower{};
    double netBuyingPower{};
    quantity_t stratGrossFilledLot{};
    double stratGrossFilledValue{};
    double totalPnl{};
    quantity_t totalOrderSent{};
    uint32_t totalFills{};
    uint32_t toalCancelSent{};
    uint32_t toalCancelAck{};
    uint32_t toalUnsolicitedCancel{};
    uint32_t toalCancelReject{};
    uint32_t totalInternalRejects{};
    uint32_t totalMarketRejects{};
    uint32_t totalLimitOrderSent{};
    uint32_t totalLimitOrderFills{};
    uint32_t maxOrderNum{};

    utils::RollingQueue srvOrderRate{};
    utils::RollingQueue srvRejectRate{};

    void setMinMax(quantity_t value) { this->minmax = value; }
    void setMinMaxValue(double value) { this->minmaxValue = value; }
    void setBuyingPower(double value) { this->buyingPower = value; }
    void setNetBuyingPower(double value) { this->netBuyingPower = value; }
    bool isHalted() const { return state.isHalted(); }
    void resetState() { state.resetTimeoutState(); }
    double getFillRatio() const
    {
        if (totalOrderSent > 0)
        {
            return (double)totalFills / totalOrderSent;
        }
        return 0.0;
    }

    void rollRates(time::NanoTime timestamp)
    {
        srvOrderRate.roll(timestamp);
        srvRejectRate.roll(timestamp);
    }

    // TODO: implement it
    // void accumulateExternalRejet(logger::Logger* logger, time::NanoTime timestamp, RejectReason reject) {}

    bool checkOrderRate(time::NanoTime timestamp, logger::Logger* logger, int32_t cid, const symbol_t& symbol,
                        Side side, price_t price, quantity_t qty)
    {
        srvOrderRate.roll(timestamp);
        if (srvOrderRate.getSize() >= maxOrderRate)
        {
            // block all orders for a timeout window
            state.setTimeout(timestamp + TimeoutWindow, StateEnum::MaxServerOrderRate);
            logger->critical("brechaed maxOrderRate:{} strategy is not allowed to send orders for {} milliseconds. set "
                             "lock state:{}",
                             maxOrderRate,
                             toString(StateEnum::MaxServerOrderRate));

            return false;
        }
        return true;
    }

    int32_t init()
    {
        auto const window = std::chrono::milliseconds(999);
        srvOrderRate.init(window);
        srvRejectRate.init(window);
        return 0;
    }
};

struct Risk
{
    Risk(const position::PortforlioPositions& positions, const std::vector<symbol_t>& symbols)
        : positions_(positions), symbols_(symbols)
    {
    }
    bool setSymbolRisk(int32_t cid, const SymbolRisk& sr)
    {
        if (symbolRisks.empty() || cid >= symbolRisks.size())
        {
            logger()->critical("failed to init risk for cid:{}", cid);
            return false;
        }
        symbolRisks[cid] = sr;
        return true;
    }
    void setRefPrice(int32_t cid, price_t price) { symbolRisks[cid].setRefPrice(price); }

    bool checkMaxOrderNum(const symbol_t& symbol, uint32_t liveOrderCount)
    {
        if (liveOrderCount >= globalRisk.maxOrderNum)
        {
            logger()->error("symbolRisk checkMaxOrderNum symbol:{} liveOrderCount:{} maxOrderNumLimit:{}",
                            symbol,
                            liveOrderCount,
                            globalRisk.maxOrderNum);
            return false;
        }
        return true;
    }

    bool isHalted(int32_t cid) const
    {
        assert(cid < symbolRisks.size());
        return symbolRisks[cid].isHalted();
    }

    bool allowOrder(time::NanoTime timestamp, int32_t cid, const symbol_t& symbol, const order_t& order,
                    const position::SymbolPosition& symPos)
    {
        if (!globalRisk.state.isGreen())
        {
            auto const state = globalRisk.state.getState(timestamp);
            logger()->error("global risk locked, state:{}", toString(state));
            return false;
        }
        auto& symRisk = symbolRisks[cid];
        if (!symRisk.state.isGreen())
        {
            auto const state = symRisk.state.getState(timestamp);
            logger()->error("symbol locked cid:{} symbol:{} state:{}", cid, symbol, toString(state));
            return false;
        }
        // TODO: implement
        // if (!checkMaxOrderNum(symbol, liveOrderCount))
        if (!symRisk.checkBasics(logger(), symbol, order))
        {
            return false;
        }
        // TODO: implement
        // if (symRisk.checkPriceDeviation)

        if (!symRisk.checkOrderRate(logger(), symbol))
        {
            return false;
        }
        if (!symRisk.checkRepeatedOrderRate(logger(), symbol, order))
        {
            return false;
        }

        if (!checkOrderRate(timestamp, symbol, order))
        {
            return false;
        }

        return false;
    }

    bool checkOrderRate(time::NanoTime timestamp, const symbol_t& symbol, const order_t& order)
    {
        if (!globalRisk.checkOrderRate(
                timestamp, logger(), order.getCid(), symbol, order.getSide(), order.getPrice(), order.getQty()))
        {
            handleInternalReject(timestamp, order, "checkOrderRate");
            return false;
        }
        return true;
    }

    void incrementInternalRejectCounter(time::NanoTime timestamp, int32_t cid) { globalRisk.totalInternalRejects++; }
    void handleInternalReject(time::NanoTime timestamp, const order_t& order, const char* desc)
    {
        incrementInternalRejectCounter(timestamp, order.getCid());
        logger()->error("internal_reject {} {}", desc, order);
    }

    /*
     * accumulate external rejects and block order sending at symbol or system for different external reasons
     */
    // TODO: implement it
    // void handelExternalReject(time::NanoTime timestamp, const order_t& order, ChangeReason reason, const char* desc)
    // {}

    void updateState(time::NanoTime timestamp)
    {
        globalRisk.rollRates(timestamp);
        tryResetGlobalState(timestamp);
        tryResetSymCounters(timestamp);
    }

    void tryResetGlobalState(time::NanoTime timestamp)
    {
        if (timestamp >= globalRisk.state.gettimeout())
        {
            auto const isPrevStateGreen = globalRisk.state.isGreen();
            globalRisk.resetState();
            if (!isPrevStateGreen)
            {
                logger()->info("");
            }
        }
    }
    void tryResetSymCounters(time::NanoTime timestamp)
    {
        for (auto& symRisk : symbolRisks)
        {
            if (symRisk.lastResetTimestamp + TimeoutWindow <= timestamp)
            {
                symRisk.resetCounters(timestamp);
            }
        }
    }

    void logGlobalRiskValue(const char* desc)
    {
        logger()->info(
            "risk::{} global_risk_values buyingPower:{} buy_minmax:{} buy_minmaxValue:{} sell_minmax:{} "
            "sell_minmaxValue:{} longValue:{} openLongPosition:{} openBuyValue:{} shortValue:{} "
            "openShortPosition:{} openSellValue:{} netPosition:{} netValue:{} stratFilledSize:{} stratFilledValue:{}",
            desc,
            positions_.getBuyingPower(),
            (positions_.getNetPosition() + positions_.getOpenLongPosition()),
            (positions_.getNetValue() + positions_.getOpenLongValue()),
            positions_.getNetPosition(),
            positions_.getOpenShortPosition(),
            (positions_.getNetValue() - positions_.getOpenShortValue()),
            positions_.getLongValue(),
            positions_.getOpenLongPosition(),
            positions_.getOpenLongValue(),
            positions_.getShortValue(),
            positions_.getOpenShortPosition(),
            positions_.getOpenShortValue(),
            positions_.getNetPosition(),
            positions_.getNetValue(),
            positions_.getStratFilledSize(),
            positions_.getStratFilledValue());
    }

    logger::Logger* logger() { return logger_; }
    int32_t init(logger::Logger* logger)
    {
        logger_ = logger;
        return 0;
    }

    logger::Logger* logger_{nullptr};
    const position::PortforlioPositions& positions_;
    const std::vector<symbol_t>& symbols_;
    GlobalRisk globalRisk{};
    std::vector<SymbolRisk> symbolRisks;
};

} // namespace miye::risk
