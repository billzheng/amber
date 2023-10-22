#pragma once

#include "libs/logger/logger.hpp"

namespace miye
{
namespace trading
{

class Strategy
{
  public:
    void onReady() { updateRefPrice(); }

    void onTrade(uint64_t timestamp);
    void onBookUpdate();

    /*
     * update reference price for risk/position model
     */
    void updateRefPrice();

    int32_t init();

  private:
    logger::Logger* logger_{nullptr};
};

inline void Strategy::updateRefPrice() {}

} // namespace trading
} // namespace miye
