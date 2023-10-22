#pragma once

#include "context.h"
#include "libcore/essential/timer.h"
#include "libcore/utils/number_utils.hpp"
#include "libs/logger/logger.hpp"

#include <chrono>
#include <iostream>

namespace miye::trading::ftx
{

enum Timers
{
    ONE_SECOND_TIMER_ID = 0,
    FIFTEEN_SECONDS_TIMER_ID,
};

constexpr static const int64_t ONE_SECOND_TIMER_INTERVAL{time::convertToNano(std::chrono::seconds(1))};
constexpr static const int64_t FIFTEEN_SECOND_TIMER_INTERVAL{time::convertToNano(std::chrono::seconds(15))};

struct TimerHandler : public NanoTimerListener
{
    explicit TimerHandler(Context& context) : context_(context) {}

    int onTimeout()
    {
        oneSecondTimer_.onTimeout();
        fifteenSecondTimer_.onTimeout();
        return 0;
    }

    int onTimer(int32_t id, time::NanoTime scheduled) override
    {
        logger()->info("onTimer id:{} {}",
                       id,
                       date::format("%F %T", std::chrono::time_point_cast<std::chrono::microseconds>(scheduled)));

        if (id == Timers::ONE_SECOND_TIMER_ID)
        {
            auto const symbol = "FTM-PERP";
            auto const info   = context_.client.get_fut_stats(symbol);

            auto const& item = info["result"];

            auto const openInterest    = item["openInterest"];
            auto const volume          = item["volume"];
            auto const nextFundingRate = item["nextFundingRate"];
            auto const nextFundingTime = item["nextFundingTime"];
            // auto const expirationPrice          = item["expirationPrice"];
            // auto const predictedExpirationPrice = item["predictedExpirationPrice"];
            logger()->info(
                "ftx future_stat symbol:{} openInterest:{} volume:{} nextFundingRate:{:03.8f} nextFundingTime:{}",
                symbol,
                openInterest,
                volume,
                nextFundingRate.get<double>(),
                nextFundingTime);
        }

        return 0;
    }

    int32_t init(logger::Logger* logger)
    {
        this->logger_ = logger;
        oneSecondTimer_.setListener(this);
        fifteenSecondTimer_.setListener(this);

        return 0;
    }

    int32_t startTimers(time::NanoTime current)
    {
        auto const start = current;
        if (oneSecondTimer_.nanoEnd() < start)
        {
            oneSecondTimer_.init(start,
                                 start + time::NanoDuration(time::convertToNano(std::chrono::hours(24))),
                                 time::NanoDuration(ONE_SECOND_TIMER_INTERVAL));
        }

        if (fifteenSecondTimer_.nanoEnd() < start)
        {
            fifteenSecondTimer_.init(start,
                                     start + time::NanoDuration(time::convertToNano(std::chrono::hours(24))),
                                     time::NanoDuration(FIFTEEN_SECOND_TIMER_INTERVAL));
        }
        return 0;
    }

    logger::Logger* logger() { return logger_; }

    logger::Logger* logger_{nullptr};
    Context& context_;

    Timer oneSecondTimer_{Timers::ONE_SECOND_TIMER_ID};
    Timer fifteenSecondTimer_{Timers::FIFTEEN_SECONDS_TIMER_ID};
};
} // namespace miye::trading::ftx
