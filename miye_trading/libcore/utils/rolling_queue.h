#pragma once

#include "libcore/utils/nano_time.h"
#include <chrono>
#include <queue>

namespace miye::utils
{

class RollingQueue
{
  private:
    std::queue<time::NanoTime> container;
    std::chrono::milliseconds window{999};

  public:
    uint32_t roll(time::NanoTime timestamp)
    {
        if (container.empty())
        {
            return 0;
        }
        uint32_t count{};
        while (!container.empty())
        {
            auto const& elem = container.front();
            if (elem + window <= timestamp)
            {
                container.pop();
                count++;
            }
            else
            {
                break;
            }
        }
        return count;
    }

    void append(time::NanoTime timestamp) { container.push(timestamp); }
    size_t getSize() const { return container.size(); }
    std::chrono::milliseconds getRollWindow() const { return window; }
    template <typename Duration>
    int32_t init(Duration window)
    {
        this->window = window;
        return 0;
    }
};

} // namespace miye::utils
