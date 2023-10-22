#pragma once
#include "libcore/essential/assert.hpp"
#include "libcore/utils/dequecontig.hpp"
#include <iostream>
#include <numeric>
#include <stdint.h>
#include <string.h>
#include <string>

// FIFO Queue
namespace miye
{
namespace utils
{

/*
 * window_size_nanos - sets the size of the window in nanoseconds from oldest
 * item in queue to newest set to zero to not limit the queue in this way
 * max_items - sets the maximum number of items in the queue
 *      set to zero to not limit the queue in this way
 *
 * one of max_items or window_size_nanos should be set
 */

struct statistics
{
    statistics() = default;
    double mean{0.0};
    double std{0.0};
};

template <typename T>
class NormalQueue
{
    uint64_t window_size_nanos;
    size_t max_items;
    T last_item_value{};
    T max_value_{};
    T min_value_{};
    bool has_last_item;
    fundamentals::dequecontig<T> items;

  public:
    NormalQueue() : window_size_nanos(0), max_items(0), has_last_item(false)
    {
        static_assert(std::is_standard_layout<T>::value,
                      "T is not standard layout");
        //::memset(&last_item_value, 0, sizeof(last_item_value));
    }

    void init(uint64_t win_sz_nanos)
    {
        items.clear();
        window_size_nanos = win_sz_nanos;
        //::memset(&last_item_value, 0, sizeof(last_item_value));
        has_last_item = false;
    }

    void set_max_size(size_t size)
    {
        max_items = size;
        items.reserve(size);
    }

    const T& get_last_item_value() const
    {
        return last_item_value;
    }
    void set_last_item_value(T val)
    {
        last_item_value = val;
        has_last_item = true;
    }

    inline void touch(uint64_t now)
    {
        if (window_size_nanos == 0)
        {
            return;
        }

        uint64_t limit = now - window_size_nanos;
        while (items.size())
        {
            auto& i = items.front();

            if (i.nanos >= limit)
            {
                break;
            }

            last_item_value = i;

            items.pop_front();
            has_last_item = true;
        }
    }

    inline void remove_oldest()
    {
        if (items.size())
        {
            auto& i = items.front();
            last_item_value = i;
            items.pop_front();
        }
    }

    inline void add(T val)
    {

        INVARIANT_MSG(!max_items || max_items >= items.size(),
                      DUMP(max_items) << DUMP(items));
        INVARIANT(max_items | window_size_nanos);
        // if theres something in the queue, newest must be older than what
        // we're adding
        INVARIANT_MSG(!items.size() || (items.back().nanos <= val.nanos),
                      DUMP(items.back().nanos) << DUMP(val.nanos));
        touch(val.nanos);

        if (max_items == items.size())
        {
            remove_oldest();
        }
        items.push_back(val);
    }

    inline size_t size() const
    {
        return items.size();
    }
    inline bool empty() const
    {
        return items.empty();
    }

    std::pair<float, float> minmax(float value) const
    {
        float min_value{value};
        float max_value{value};

        auto const len = size();
        for (size_t i = 0; i < len; ++i)
        {
            if (!std::isnan(items[i].value))
            {
                if (items[i].value < min_value)
                {
                    min_value = items[i].value;
                    continue;
                }
                if (items[i].value > max_value)
                {
                    max_value = items[i].value;
                    continue;
                }
            }
        }

        return std::make_pair(min_value, max_value);
    }

    static statistics standardDeviation(std::vector<double>& v)
    {
        statistics stat{};
        double sum = std::accumulate(v.begin(), v.end(), 0.0);
        stat.mean = sum / v.size();

        double squareSum =
            std::inner_product(v.begin(), v.end(), v.begin(), 0.0);
        stat.std = std::sqrt(squareSum / v.size() - stat.mean * stat.mean);

        return stat;
    }

    statistics stats() const
    {
        auto const len = size();
        std::vector<double> v;
        v.reserve(len);

        for (size_t i = 0; i < len; ++i)
        {
            v.push_back(items[i].value);
        }

        return standardDeviation(v);
    }

    inline T head() const
    {
        if (items.size() > 0)
        {
            return items.back();
        }
        return T{};
    }

    inline T tail() const
    {
        if (items.size() > 0)
        {
            return items.front();
        }
        return T{};
    }

    inline T first() const
    {
        if (items.size() > 0)
        {
            return items.back();
        }
        return last_item_value;
    }

    inline T last() const
    {
        if (has_last_item)
        {
            return last_item_value;
        }
        return tail();
    }

    inline bool has_last() const
    {
        return has_last_item;
    }

    inline T operator[](size_t index) const
    {
        if (index > items.size())
        {
            return T{};
        }
        return items[index];
    }

    inline uint64_t window_size()
    {
        if (items.size() > 0)
        {
            return items.back().nanos - items.front().nanos;
        }
        else
        {
            return 0;
        }
    }

    void dump(uint64_t now, std::string tag)
    {
        std::cout << tag << std::endl;
        std::cout << "Queue window_size_nanos=" << window_size_nanos
                  << ", max_items=" << max_items << ",size=" << items.size()
                  << std::endl;
        std::cout << "timestamp,value,age" << std::endl;
        for (auto& o : items)
        {
            std::cout << o.nanos << "," << o << "," << (now - o.nanos)
                      << std::endl;
        }
    }
};

} // namespace utils
} // namespace miye
