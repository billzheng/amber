#pragma once
#include "libcore/math/math_utils.hpp"
#include "libcore/types/types.hpp"
#include "spdlog/common.h"

namespace miye
{
namespace trading
{

#pragma pack(push, 1)

struct BBO
{
    price_t bidPx{};
    price_t askPx{};
    quantity_t bidQty{};
    quantity_t askQty{};

    BBO() = default;
    double mid() const noexcept
    {
        if (isValid())
        {
            return (bidPx + askPx) / 2;
        }
        return NAN;
    }

    double smid() const noexcept
    {
        if (isValid())
        {
            return (bidPx * askQty + askPx * bidQty) / (bidQty + askQty);
        }
        return NAN;
    }

    bool isLocked() const
    {
        return math::floatEqual(bidPx, askPx);
    }
    bool isCrossed() const
    {
        return math::lessEqual(askPx, bidPx);
    }
    bool isValid() const
    {
        return bidQty > 0 && askQty > 0 && !std::isnan(bidPx) &&
               !std::isnan(askPx) && !isCrossedOrLocked();
    }
    std::string toString() const
    {
        std::stringstream ss;
        ss << std::fixed << "bidPx:" << bidPx << " bidQty:" << bidQty
           << " askPx:" << askPx << " askQty:" << askQty << " mid:" << mid()
           << " smid:" << smid() << " isvalid:" << isValid();
        return ss.str();
    }

  private:
    // book is locked when bidPx == askPx
    // boook is crossed when bidPx > askPx
    bool isCrossedOrLocked() const
    {
        return math::lessEqual(askPx, bidPx);
    }
};

template <typename OS>
inline OS& operator<<(OS& os, const BBO& o)
{
    os << std::fixed << "bidPx:" << o.bidPx << " bidQty:" << o.bidQty
       << " askPx:" << o.askPx << " askQty:" << o.askQty << " mid:" << o.mid()
       << " smid:" << o.smid() << " isvalid:" << o.isValid();
    return os;
}

#pragma pack(pop)
} // namespace trading
} // namespace miye

namespace fmt_lib = spdlog::fmt_lib;
template <>
struct fmt_lib::formatter<miye::trading::BBO> : fmt_lib::formatter<std::string>
{
    auto format(const miye::trading::BBO& bbo, format_context& ctx)
        -> decltype(ctx.out())
    {
        return fmt_lib::format_to(ctx.out(),
                                  "bbo bidPx={} bidQty={} askPx={} asxQty={} "
                                  "mikd={} smid={} is_valid={}",
                                  bbo.bidPx,
                                  bbo.bidQty,
                                  bbo.askPx,
                                  bbo.askQty,
                                  bbo.mid(),
                                  bbo.smid(),
                                  bbo.isValid());
    }
};
