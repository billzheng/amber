#pragma once
#include "libcore/types/types.hpp"
#include "libcore/utils/nano_time.h"
#include "libs/logger/logger.hpp"
namespace miye
{
namespace position
{

struct SymbolPosition
{
    price_t markPx{};
    double multiplier{1.0};
    quantity_t buyQty{0.0};
    quantity_t sellQty{0.0};
    price_t buyValue{0.0};
    price_t sellValue{0.0};

    quantity_t openBuyQty{0.0};
    price_t openBuyValue{0.0};
    quantity_t openSellQty{0.0};
    price_t openSellValue{0.0};

    quantity_t yesterdayLong{0.0};
    quantity_t yesterdayShort{0.0};
    price_t yesterdayLongValue{0.0};
    price_t yesterdayShortValue{0.0};

    quantity_t todaySell{0.0};
    price_t todaySellValue{0.0};
    quantity_t todayBuy{0.0};
    price_t todayBuyValue{0.0};

    int32_t numBuyOrder{};
    int32_t numSellOrder{};
    int32_t getNumBuyOrder() const { return numBuyOrder; }
    int32_t getNumSellOrder() const { return numSellOrder; }
    quantity_t getNetPosition() const { return buyQty - sellQty; }
    price_t getNetValue() const { return getNetPosition() * this->markPx; }
    bool hasLiveOrders() const { return numBuyOrder > 0 || numSellOrder > 0; }
    void setMarkPx(double v) { markPx = v; }

    quantity_t getDayNetPosition() const { return todayBuy - todaySell; }
    quantity_t getDayFilledQty() const { return todayBuy + todaySell; }
    quantity_t getDayFilledValue() const { return todayBuyValue + todaySellValue; }

    quantity_t getOpenBuy() const { return openBuyQty; }
    quantity_t getOpenSell() const { return openSellQty; }
    price_t getOpenBuyValue() const { return openBuyValue; }
    price_t getOpenSellValue() const { return openSellValue; }

    quantity_t getLongPosition() const { return std::max(0.0, getNetPosition()); }
    quantity_t getShortPosition() const { return std::max(0.0, -1 * getNetPosition()); }

    price_t getLongValue() const { return std::max(0.0, getNetValue()); }
    price_t getShortValue() const { return std::max(0.0, -1 * getNetValue()); }

    double getNetBuyingPower() const { return std::fabs(getLongValue() + getShortValue()); }
    double getBuyingPower() const
    {
        auto const netValue = getNetValue();
        return std::max(netValue + openBuyValue, -netValue + openSellValue);
    }
    double getDayPnl() const { return getNetPosition() * markPx - (todayBuyValue - todaySellValue); }

    void onFill(price_t price, quantity_t qty)
    {
        buyQty += qty;
        buyValue += price * qty;
    }

    void onPlaceOrder();
    void onCancelOrder();
    void onReject();
};

class PortforlioPositions
{
  public:
    logger::Logger* logger_{nullptr};

  private:
    std::unordered_map<symbol_t, SymbolPosition> aggregatedPositions;
    double netBuyingPower_{};
    double buyingPower_{};
    double longValue_{};
    double shortValue_{};
    quantity_t longPos_{};
    quantity_t shortPos_{};

    quantity_t openLongPos_{};
    quantity_t openShortPos_{};

    double openLongValue_{};
    double openShortValue_{};

    quantity_t netPos_;
    double netValue_;
    quantity_t dayFilledSize_{};
    double dayFilledValue_{};

    quantity_t numLiveBuyOrder_{};
    quantity_t numLiveSellOrder_{};

    quantity_t stratFilledSize_{};
    double stratFilledValue_{};

  public:
    SymbolPosition* getSymPos(const symbol_t& symbol);
    const SymbolPosition* getSymPos(const symbol_t& symbol) const;

    double getNetBuyingPower() const { return netBuyingPower_; }
    double getBuyingPower() const { return buyingPower_; }

    void placeOrder(const symbol_t& symbol, Side side, price_t price, quantity_t qty);
    void onReject(const symbol_t& symbol, Side side, price_t price, quantity_t qty);

    void onFill(const symbol_t& symbol, Side side, price_t fillPrice, quantity_t fillQty);

    logger::Logger* logger() { return logger_; }
    const std::unordered_map<symbol_t, SymbolPosition>& getAggregatedPositions() const { return aggregatedPositions; }

    quantity_t getDayFilledSize() const { return dayFilledSize_; }

    double getDayFilledValue() const { return dayFilledValue_; }

    quantity_t getLongPosition() const { return longPos_; }

    double getLongValue() const { return longValue_; }

    quantity_t getNetPosition() const { return netPos_; }

    double getNetValue() const { return netValue_; }

    quantity_t getNumLiveBuyOrder() const { return numLiveBuyOrder_; }

    quantity_t getNumLiveSellOrder() const { return numLiveSellOrder_; }

    quantity_t getOpenLongPosition() const { return openLongPos_; }

    double getOpenLongValue() const { return openLongValue_; }

    quantity_t getOpenShortPosition() const { return openShortPos_; }

    double getOpenShortValue() const { return openShortValue_; }

    quantity_t getShortPosition() const { return shortPos_; }

    double getShortValue() const { return shortValue_; }

    quantity_t getStratFilledSize() const { return stratFilledSize_; }

    double getStratFilledValue() const { return stratFilledValue_; }

    void setMarkPrice(const symbol_t& symbol, price_t price)
    {
        auto it = aggregatedPositions.find(symbol);
        if (it != aggregatedPositions.end())
        {
            auto& symPos = it->second;
            symPos.setMarkPx(price);
        }
    }

    int32_t init(logger::Logger* logger)
    {
        logger_ = logger;
        return 0;
    }

  private:
};

inline SymbolPosition* PortforlioPositions::getSymPos(const symbol_t& symbol)
{
    auto it = aggregatedPositions.find(symbol);
    if (it != aggregatedPositions.end())
    {
        return &it->second;
    }
    return nullptr;
}
inline const SymbolPosition* PortforlioPositions::getSymPos(const symbol_t& symbol) const
{
    auto const it = aggregatedPositions.find(symbol);
    if (it != aggregatedPositions.end())
    {
        return &it->second;
    }
    return nullptr;
}

inline void PortforlioPositions::placeOrder(const symbol_t& symbol, Side side, price_t price, quantity_t qty)
{
    auto* pos = getSymPos(symbol);
    if (pos)
    {
        auto const orderValue         = price * qty;
        auto const prevSymBuyingPower = pos->getBuyingPower();

        if (side == Side::BUY)
        {
            pos->openBuyQty += qty;
            pos->openBuyValue += orderValue;
            pos->numBuyOrder++;
        }
        else
        {
            pos->openSellQty += qty;
            pos->openSellValue += orderValue;
            pos->numSellOrder++;
        }
        buyingPower_ += pos->getBuyingPower() - prevSymBuyingPower;
    }
    else
    {
        logger()->error("position not found for symbol:{}", symbol);
    }
}

inline void PortforlioPositions::onReject(const symbol_t& symbol, Side side, price_t price, quantity_t qty)
{
    auto* pos = getSymPos(symbol);
    if (pos)
    {
        auto const orderValue         = price * qty;
        auto const prevSymBuyingPower = pos->getBuyingPower();

        if (side == Side::BUY)
        {
            pos->openBuyQty -= qty;
            pos->openBuyValue -= orderValue;
            pos->numBuyOrder--;
        }
        else
        {
            pos->openSellQty -= qty;
            pos->openSellValue = orderValue;
            pos->numSellOrder--;
        }
        buyingPower_ += pos->getBuyingPower() - prevSymBuyingPower;
        logger()->info("position::onReject symbol:{} revert order position "
                       "side:{} price:{} qty:{}",
                       symbol,
                       toString(side),
                       price,
                       qty);
    }
    else
    {
        logger()->critical("position not found for symbol:{}", symbol);
    }
}
/*
 * TODO: handle full fill and partial fill
 */
inline void PortforlioPositions::onFill(const symbol_t& symbol, Side side, price_t fillPrice, quantity_t fillQty) {}

} // namespace position
} // namespace miye
