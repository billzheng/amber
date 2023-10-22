#pragma once
#include "libcore/math/math_utils.hpp"
#include "libcore/types/types.hpp"

#include <algorithm>
#include <functional>

namespace miye
{
namespace trading
{

#pragma pack(push, 1)
//
// enum class TradeDir : int16_t
//{
//    NONE,
//    BUY,
//    SELL
//};
//
// enum class Side : uint8_t
//{
//    BUY = 0,
//    SELL,
//    SHORT_SELL
//};

template <typename Price = price_t>
struct PriceLevel
{

    PriceLevel() = default;
    PriceLevel(Price price, quantity_t quantity)
        : price_(price), quantity_(quantity)
    {
    }

  public:
    Price getPrice() const noexcept
    {
        return price_;
    }
    void setPrice(Price price) noexcept
    {
        price_ = price;
    }

    quantity_t getQuantity() const noexcept
    {
        return quantity_;
    }
    void setQuantity(quantity_t quantity) noexcept
    {
        quantity_ = quantity;
    }

    friend bool operator==(const PriceLevel<Price>& lhs,
                           const PriceLevel<Price>& rhs)
    {
        return lhs.getQuantity() == rhs.getQuantity() &&
               math::floatEqual(lhs.getPrice(), rhs.getPrice());
    }

    bool isValid() const
    {
        return !std::isnan(quantity_) && !std::isnan(price_) && quantity_ > 0.0;
    }

    template <typename OS>
    friend OS& operator<<(OS& os, const PriceLevel& o)
    {
        os << "price:" << o.getPrice() << " quantity:" << o.getQuantity();
        return os;
    }

  private:
    Price price_{};
    quantity_t quantity_{};
};

/*
 * use vector to quickly build orderbook, improve the performance later
 */
template <typename Price = price_t>
class BookSide
{
  public:
    size_t getLevels() noexcept
    {
        return levels_.size();
    }

    //    void clear()
    //    {
    //        levels_.clear();
    //    }
    void init(int32_t BookDepth)
    {
        levels_.resize(BookDepth);
    }

    const PriceLevel<Price>& getLevel(size_t lvlIdx) const noexcept
    {
        auto const lvlNum = levels_.size();
        assert(lvlIdx < levels_.size());
        return levels_[lvlNum - lvlIdx - 1];
    }

    void removeLevel(Price price)
    {

        auto it =
            std::find_if(levels_.begin(),
                         levels_.end(),
                         [price](const PriceLevel<Price>& lvl) {
                             return math::floatEqual(lvl.getPrice(), price);
                         });

        if (it == levels_.end())
        {
            std::cout << "remove level:" << price
                      << " found:" << (it != levels_.end()) << std::endl;
        }
        if (it != levels_.end())
        {
            levels_.erase(it);
        }
    }

    //    void setOrInsertBidLevel(Price price, quantity_t quantity)
    //    {
    //        if (levels_.empty() || price > levels_.back().getPrice())
    //        {
    //            levels_.push_back(PriceLevel<Price>{price, quantity});
    //            // std::cout << "bookside:" << toString() << std::endl;
    //            return;
    //        }
    //        for (int32_t i = levels_.size() - 1; i >= 0; i--)
    //        {
    //            auto& lvl = levels_[i];
    //            if (price == lvl.getPrice())
    //            {
    //                lvl.setPrice(price);
    //                lvl.setQuantity(quantity);
    //                return;
    //            }
    //            else if (price > lvl.getPrice())
    //            {
    //                levels_.insert(levels_.begin() + i,
    //                               PriceLevel<Price>{price, quantity});
    //                // std::cout << "bookside:" << toString() << std::endl;
    //                return;
    //            }
    //        }
    //
    //        levels_.insert(levels_.begin(), PriceLevel<Price>{price,
    //        quantity});
    //        // std::cout << "bookside:" << toString() << std::endl;
    //    }

    void setOrInsertLevel(Price price, quantity_t quantity)
    {
        if (levels_.empty() || op_(price, levels_.back().getPrice()))
        {
            levels_.push_back(PriceLevel<Price>{price, quantity});
            // std::cout << "bookside:" << toString() << std::endl;
            return;
        }
        for (int32_t i = levels_.size() - 1; i >= 0; i--)
        {
            auto& lvl = levels_[i];
            if (price == lvl.getPrice())
            {
                // lvl.setPrice(price);
                lvl.setQuantity(quantity);
                return;
            }
            else if (op_(price, lvl.getPrice()))
            {
                levels_.insert(levels_.begin() + i + 1,
                               PriceLevel<Price>{price, quantity});
                // std::cout << "bookside:" << toString() << std::endl;
                return;
            }
        }

        levels_.insert(levels_.begin(), PriceLevel<Price>{price, quantity});
        // std::cout << "bookside:" << toString() << std::endl;
    }

    void setLevel(int32_t idx, Price price, quantity_t quantity)
    {
        assert(idx < levels_.size());
        levels_[idx] = PriceLevel<Price>{price, quantity};
    }

    bool isValid() const
    {
        if (levels_.empty())
        {
            return true;
        }
        if (levels_.size() == 1)
        {

            return levels_.back().isValid();
        }

        for (int32_t i = levels_.size() - 1, j = levels_.size() - 2; j >= 0;
             --i, --j)
        {
            auto const lvl_i = levels_[i];
            auto const lvl_j = levels_[j];
            if (!lvl_i.isValid() || !lvl_j.isValid() ||
                !op_(lvl_i.getPrice(), lvl_j.getPrice()))
            {

                std::cout << "level is not valid:" << lvl_i << ' ' << lvl_j
                          << std::endl;

                return false;
            }
        }
        return true;
    }

    const PriceLevel<Price>* getTopLevel() const
    {
        if (levels_.empty())
        {
            return nullptr;
        }
        return &levels_.back();
    }

    /*
     *
     */
    void setPriceOp(std::function<bool(Price, Price)> op)
    {
        op_ = op;
    }

    std::string toString() const
    {
        std::stringstream ss;
        for (int32_t i = levels_.size() - 1; i >= 0; --i)
        {
            auto const& lvl = levels_[i];
            ss << std::fixed << lvl.getPrice() << '@' << lvl.getQuantity()
               << '\n';
        }
        return ss.str();
    }

  private:
    std::vector<PriceLevel<Price>> levels_{};
    std::function<bool(Price, Price)> op_;
};

#pragma pack(pop)

} // namespace trading
} // namespace miye
