#pragma once
#include "exchange.h"
#include "order_book.h"

#include <algorithm>
#include <functional>
#include <vector>

namespace miye
{
namespace trading
{

#pragma pack(push, 1)

/*
 * CID is the index of the symbol in symbols_ vector
 *
 */
template <size_t SYM_SIZE>
class OrderBookStore
{
  public:
    OrderBookStore()
    {
        orderBooks_.resize(SYM_SIZE);
        symbols_.resize(SYM_SIZE);
        assert(orderBooks_.size() == symbols_.size());
    }
    OrderBook& getBook(Exchange exchange, const std::string& symbol)
    {
        auto const cid = getCid(exchange, symbol);
        assert(cid != INVALID_CID);
        return orderBooks_[cid];
    }

    OrderBook& getBook(int32_t cid) { return orderBooks_[cid]; }
    const OrderBook& getBook(int32_t cid) const { return orderBooks_[cid]; }

    static symbol_t buildSymbol(Exchange exchange, const std::string& symbol)
    {
        return std::string(toString(exchange)) + ':' + symbol;
    }

    int32_t getCid(Exchange exchange, const std::string& symbol)
    {
        auto const fullSymbol = buildSymbol(exchange, symbol);
        auto const cid        = getCid(fullSymbol);
        //        std::cout << "fullsymbol:" << fullSymbol << " cid:" << cid << std::endl;
        return cid;
    }

    const symbol_t& getSymbol(int32_t cid) const { return symbols_[cid]; }

    int32_t getCid(const std::string& symbol) const
    {
        auto const it = std::find(symbols_.begin(), symbols_.end(), symbol);
        if (it != symbols_.end())
        {
            return std::distance(symbols_.begin(), it);
        }
        return INVALID_CID;
    }

    int32_t initBook(Exchange exchange, int32_t BookDepth)
    {
        const std::string exchStr = std::string(toString(exchange)) + ":";
        for (int32_t cid = 0; cid < symbols_.size(); cid++)
        {
            auto const& symbol = symbols_[cid];
            if (symbol.find(exchStr) != std::string::npos)
            {
                orderBooks_[cid].init(BookDepth);
            }
            else
            {
                std::cout << "ERROR book not found for symbol:" << symbol << std::endl;
            }
        }
        return 0;
    }

    void setSymbols(const std::vector<symbol_t>& symbols)
    {
        assert(symbols.size() == symbols_.size());
        symbols_ = symbols;
    }
    size_t getSymbolNum() const
    {
        assert(orderBooks_.size() == symbols_.size());
        assert(orderBooks_.size() > 0);
        return orderBooks_.size();
    }

    const std::vector<symbol_t>& getSymbols() const { return symbols_; }

  public:
    std::vector<OrderBook> orderBooks_{};
    /*
     * symbol format: EXCHANGE:SYMBOL. FTM-PERP on
     * FTX is FTX:FTM-PERP
     * BINANCE is BINANCE:FTMUSDT
     */
    std::vector<symbol_t> symbols_{};
};

#pragma pack(pop)

} // namespace trading
} // namespace miye
