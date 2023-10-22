#pragma once
#include "libcore/math/math_util.h"
#include "libcore/types/types.hpp"
#include "libcore/utils/queue.hpp"
#include "trading.h"

#include <vector>

namespace miye
{
namespace trading
{

template <typename T = double>
struct TradeItem
{
    T qty{};
    TradeItem() = default;

    explicit TradeItem(T v) : qty(v)
    {
    }

    TradeItem& operator+=(const TradeItem& rhs)
    {
        qty += rhs.qty;
        return *this;
    }
    TradeItem& operator-=(const TradeItem& rhs)
    {
        qty -= rhs.qty;
        return *this;
    }
    TradeItem& operator=(const TradeItem& rhs)
    {
        this->qty = rhs.qty;
        return *this;
    }
    bool operator==(const TradeItem& rhs) const
    {
        return math::floatEqual(this->qty, rhs.qty);
    }
};

template <typename OS, typename T>
OS& operator<<(OS& os, TradeItem<T> const& item)
{
    os << item.qty;
    return os;
}

class TradeFlow
{
  public:
    Signal onTrade(uint64_t timestamp, const std::vector<trade_t>& trades);
    void onBookChange();

    void addFlow(uint64_t timestamp, const trade_t& trade)
    {
        auto const tradeDir =
            trade.side == Side::BUY ? TradeDir::BUY : TradeDir::SELL;

        tradeFlow_.add(timestamp, TradeItem<double>(trade.qty * tradeDir));
    }

    void decay(uint64_t timestamp)
    {
        tradeFlow_.touch(timestamp);
    }

    Signal calc()
    {
        Signal signal{};
        auto const stateItem = tradeFlow_.sum();

        double absSum{0.0};
        for (int i = 0; i < tradeFlow_.size(); ++i)
        {
            auto const item = tradeFlow_[i];
            absSum += abs(item.qty);
        }

        auto const rawAlpha = double(stateItem.qty) / double(absSum);
        //        std::cout << "sum:" << stateItem.qty << " abs_sum:" << absSum
        //                  << " rawAlpha:" << rawAlpha << std::endl;
        signal.setValue(rawAlpha);
        return signal;
    }

    int32_t init(uint64_t rollWindowNs)
    {
        tradeFlow_.init(rollWindowNs);
        return 0;
    }

  private:
    utils::Queue<TradeItem<double>> tradeFlow_;
};

inline Signal TradeFlow::onTrade(uint64_t timestamp,
                                 const std::vector<trade_t>& trades)
{
    tradeFlow_.touch(timestamp);
    for (auto const& trade : trades)
    {
        addFlow(timestamp, trade);
    }
    return calc();
}

} // namespace trading
} // namespace miye
