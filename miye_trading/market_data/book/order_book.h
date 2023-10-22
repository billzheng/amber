#pragma once
#include "bbo.h"
#include "book_side.hpp"
#include "libcore/math/math_utils.hpp"
#include "libcore/types/types.hpp"

#include <algorithm>
#include <functional>

namespace miye
{
namespace trading
{

#pragma pack(push, 1)

class OrderBook
{
  public:
    OrderBook()
    {
        std::cout << "orderbook set op for bid/ask" << std::endl;
        auto& bidSide = sides_[to_underlying(Side::BUY)];
        bidSide.setPriceOp(std::greater<price_t>());
        auto& askSide = sides_[to_underlying(Side::SELL)];
        askSide.setPriceOp(std::less<price_t>());
    }

  public:
    void setOrInsertLevel(Side side, price_t price, quantity_t quantity)
    {
        auto& bookSide = sides_[to_underlying(side)];
        bookSide.setOrInsertLevel(price, quantity);

        //        if (side == Side::BUY)
        //        {
        //            auto& bidBookSide = sides_[to_underlying(side)];
        //            bidBookSide.setOrInsertBidLevel(price, quantity);
        //        }
        //        else if (side == Side::SELL)
        //        {
        //            auto& askBookSide = sides_[to_underlying(side)];
        //            askBookSide.setOrInsertAskLevel(price, quantity);
        //        }
    }

    void removeLevel(Side side, price_t price)
    {
        auto& bookSide = sides_[to_underlying(side)];
        bookSide.removeLevel(price);
    }

    void setLevel(Side side, int32_t idx, price_t price, quantity_t quantity)
    {
        auto& bookSide = sides_[to_underlying(side)];
        bookSide.setLevel(idx, price, quantity);
    }

    const PriceLevel<price_t>& getLevel(Side side, int32_t idx) const
    {
        // assert(idx < bookDepth_);
        auto& bookSide = sides_[to_underlying(side)];
        return bookSide.getLevel(idx);
    }

    //    void clearSide(Side side)
    //    {
    //        auto& bookSide = sides_[to_underlying(side)];
    //        bookSide.clear();
    //    }
    //
    //    void clear()
    //    {
    //        clearSide(Side::BUY);
    //        clearSide(Side::SELL);
    //    }

    BBO getBBO() const
    {
        BBO bbo{};

        auto const& bidSide     = sides_[to_underlying(Side::BUY)];
        auto const* topBidLevel = bidSide.getTopLevel();
        if (topBidLevel)
        {
            bbo.bidPx  = topBidLevel->getPrice();
            bbo.bidQty = topBidLevel->getQuantity();
        }

        auto const& askSide     = sides_[to_underlying(Side::SELL)];
        auto const* topAskLevel = askSide.getTopLevel();
        if (topAskLevel)
        {
            bbo.askPx  = topAskLevel->getPrice();
            bbo.askQty = topAskLevel->getQuantity();
        }
        return bbo;
    }

    void setLastTrade(uint64_t timestamp, uint64_t id, const trade_t& trade) { this->lastTrade_ = trade; }
    void setLastTrade(uint64_t timestamp, uint64_t id, Side side, price_t price, quantity_t qty)
    {
        this->lastTradeTimestamp_ = timestamp;
        this->tradeId_            = id;
        this->lastTrade_.side     = side;
        this->lastTrade_.price    = price;
        this->lastTrade_.qty      = qty;
    }

    bool isValid() const
    {
        // check prices are in order
        // check book is not crossed
        // check checksum is valid
        auto& bidSide = sides_[to_underlying(Side::BUY)];
        auto& askSide = sides_[to_underlying(Side::SELL)];

        auto const bbo = getBBO();
        return bidSide.isValid() && askSide.isValid() && bbo.isValid();
    }

    void print() const
    {
        //        auto& bidSide = sides_[to_underlying(Side::BUY)];
        //        auto& askSide = sides_[to_underlying(Side::SELL)];
        //        //        std::cout << "bid \n" << bidSide.toString() <<
        //        std::endl;
        //        //        std::cout << "ask \n" << askSide.toString() <<
        //        std::endl; auto const bbo = getBBO();
        //        // std::cout << "bbo " << bbo << std::endl;
    }

    void init(uint32_t BookDepth)
    {
        auto& bidSide = sides_[to_underlying(Side::BUY)];
        bidSide.init(BookDepth);
        auto& askSide = sides_[to_underlying(Side::SELL)];
        askSide.init(BookDepth);
    }

  private:
    std::array<BookSide<price_t>, 2> sides_;
    trade_t lastTrade_{};
    uint64_t tradeId_{};
    uint64_t lastTradeTimestamp_{};
};

#pragma pack(pop)

} // namespace trading
} // namespace miye

namespace fmt_lib = spdlog::fmt_lib;
template <>
struct fmt_lib::formatter<miye::trading::OrderBook> : fmt_lib::formatter<std::string>
{
    using Side = miye::Side;

    auto format(const miye::trading::OrderBook& book, format_context& ctx) -> decltype(ctx.out())
    {
        auto const& bid_lvl_0 = book.getLevel(Side::BUY, 0);
        auto const& bid_lvl_1 = book.getLevel(Side::BUY, 1);
        auto const& bid_lvl_2 = book.getLevel(Side::BUY, 2);
        auto const& bid_lvl_3 = book.getLevel(Side::BUY, 3);
        auto const& bid_lvl_4 = book.getLevel(Side::BUY, 4);

        auto const& ask_lvl_0 = book.getLevel(Side::SELL, 0);
        auto const& ask_lvl_1 = book.getLevel(Side::SELL, 1);
        auto const& ask_lvl_2 = book.getLevel(Side::SELL, 2);
        auto const& ask_lvl_3 = book.getLevel(Side::SELL, 3);
        auto const& ask_lvl_4 = book.getLevel(Side::SELL, 4);

        return fmt_lib::format_to(ctx.out(),
                                  "{:0.2f}@{:0.4f},{:0.2f}@{:0.4f},{:0.2f}@{:0.4f},{:0.2f}@{:0.4f},{:"
                                  "0.2f}@{:0.4f},{:0.2f}@{:0.4f},{:0.2f}@{:0.4f},{:0.2f}@{:0.4f},{:0."
                                  "2f}@{:0.4f},{:0.2f}@{:0.4f},{}",
                                  bid_lvl_0.getQuantity(),
                                  bid_lvl_0.getPrice(),
                                  bid_lvl_1.getQuantity(),
                                  bid_lvl_1.getPrice(),
                                  bid_lvl_2.getQuantity(),
                                  bid_lvl_2.getPrice(),
                                  bid_lvl_3.getQuantity(),
                                  bid_lvl_3.getPrice(),
                                  bid_lvl_4.getQuantity(),
                                  bid_lvl_4.getPrice(),
                                  ask_lvl_0.getQuantity(),
                                  ask_lvl_0.getPrice(),
                                  ask_lvl_1.getQuantity(),
                                  ask_lvl_1.getPrice(),
                                  ask_lvl_2.getQuantity(),
                                  ask_lvl_2.getPrice(),
                                  ask_lvl_3.getQuantity(),
                                  ask_lvl_3.getPrice(),
                                  ask_lvl_4.getQuantity(),
                                  ask_lvl_4.getPrice(),
                                  book.isValid());
    }
};
