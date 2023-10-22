#pragma once
#include "libcore/types/types.hpp"
#include "spdlog/common.h"

namespace miye
{
namespace trading
{

struct instrument_t
{
    symbol_t symbol;
    price_t tickSize{}; // price step
    quantity_t qtyStep{};
    double imfFactor{};
    int32_t positionLimitWgt{};
};

template <typename OS>
inline OS& operator<<(OS& os, const instrument_t& o)
{
    os << "symbol:" << o.symbol << " tickSize:" << o.tickSize
       << " qtyStep:" << o.qtyStep << " imfFactor:" << o.imfFactor
       << " positionLimitWgt:" << o.positionLimitWgt;
    return os;
}

} // namespace trading
} // namespace miye

namespace fmt_lib = spdlog::fmt_lib;
template <>
struct fmt_lib::formatter<miye::trading::instrument_t>
    : fmt_lib::formatter<std::string>
{
    auto format(const miye::trading::instrument_t& i, format_context& ctx)
        -> decltype(ctx.out())
    {
        return fmt_lib::format_to(
            ctx.out(),
            "symbol:{} tickSize{} qtyStep:{} imfFactor:{} positionLimitWgt:{}",
            i.symbol,
            i.tickSize,
            i.qtyStep,
            i.imfFactor,
            i.positionLimitWgt);
    }
};
