#pragma once
#include "libcore/essential/assert.hpp"
#include "libcore/utils/dequecontig.hpp"
#include <iostream>
#include <stdint.h>
#include <string.h>
#include <string>

// FIFO Queue
namespace miye
{
namespace utils
{

// generic queue entry
template <typename A, typename B>
struct tf_item
{
    A first;
    B second;

    tf_item(A a, B b) : first(a), second(b)
    {
    }

    tf_item()
    {
        static_assert(std::is_standard_layout<A>::value,
                      "A is not standard layout");
        static_assert(std::is_standard_layout<B>::value,
                      "B is not standard layout");
        ::memset(&first, 0, sizeof(first));
        ::memset(&second, 0, sizeof(second));
    }

    tf_item& operator+=(const tf_item& rhs)
    {
        first += rhs.first;
        second += rhs.second;
        return *this;
    }
    tf_item& operator-=(const tf_item& rhs)
    {
        first -= rhs.first;
        second -= rhs.second;
        return *this;
    }
    tf_item& operator=(const tf_item& rhs)
    {
        first = rhs.first;
        second = rhs.second;
        return *this;
    }
    bool operator==(const tf_item& rhs) const
    {
        return first == rhs.first && second == rhs.second;
    }
};

template <typename OS, typename A, typename B>
OS& operator<<(OS& os, tf_item<A, B> t)
{
    os << "tf_item { \"first\": " << t.first << ", \"second\": " << t.second
       << " }";
    return os;
}

template <typename T>
struct Item
{
    explicit Item(uint64_t nanos_, T val_) : nanos(nanos_), val(val_)
    {
    }

    explicit Item() : nanos(0), val()
    {
    }
    bool operator==(const Item& rhs)
    {
        return nanos == rhs.nanos && val == rhs.val;
    }

    uint64_t nanos;
    T val;
};
template <typename OS, typename T>
OS& operator<<(OS& os, Item<T> const& item)
{
    os << "{ \"nanos\": " << item.nanos << ", \"val\": " << item.val << " }";
    return os;
}

/*
 * window_size_nanos - sets the size of the window in nanoseconds from oldest
 * item in queue to newest set to zero to not limit the queue in this way
 * max_items - sets the maximum number of items in the queue
 *      set to zero to not limit the queue in this way
 *
 * one of max_items or window_size_nanos should be set
 */

template <typename T, typename E = T>
class Queue
{
    T state_sum{};
    // T abs_state_sum{};
    uint64_t window_size_nanos;
    size_t max_items;
    T last_item_value;
    bool has_last_item;
    fundamentals::dequecontig<Item<E>> items;

  public:
    Queue() : window_size_nanos(0), max_items(0), has_last_item(false)
    {
        static_assert(std::is_standard_layout<T>::value,
                      "T is not standard layout");
        ::memset(&state_sum, 0, sizeof(state_sum));
        ::memset(&last_item_value, 0, sizeof(last_item_value));
    }

    void init(uint64_t win_sz_nanos)
    {
        items.clear();
        window_size_nanos = win_sz_nanos;
        ::memset(&state_sum, 0, sizeof(state_sum));
        ::memset(&last_item_value, 0, sizeof(last_item_value));
        has_last_item = false;

        std::cout << "win_sz_nanos:" << win_sz_nanos << std::endl;
    }

    void set_max_size(size_t size)
    {
        max_items = size;
        items.reserve(size);
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

            state_sum -= i.val;
            // abs_state_sum -= abs(i.val);
            last_item_value = i.val;
            items.pop_front();
            has_last_item = true;
        }
    }

    inline void remove_oldest()
    {
        if (items.size())
        {
            auto& i = items.front();
            state_sum -= i.val;
            // abs_state_sum -= abs(i.val);
            last_item_value = i.val;
            items.pop_front();
        }
    }

    inline void add(uint64_t now, T val)
    {

        INVARIANT_MSG(!max_items || max_items >= items.size(),
                      DUMP(max_items) << DUMP(items));
        INVARIANT(max_items | window_size_nanos);
        // if theres something in the queue, newest must be older than what
        // we're adding
        INVARIANT_MSG(!items.size() || (items.back().nanos <= now),
                      DUMP(items.back().nanos) << DUMP(now));
        touch(now);

        if (max_items == items.size())
        {
            remove_oldest();
        }

        state_sum += val;
        //        abs_state_sum += abs(val);
        Item<T> item(now, val);
        items.push_back(item);
    }

    inline size_t size() const
    {
        return items.size();
    }
    inline bool empty() const
    {
        return items.empty();
    }

    inline T sum() const
    {
        return state_sum;
    }
    //    inline T absSum() const
    //    {
    //        return abs_state_sum;
    //    }

    inline T head() const
    {
        if (items.size() > 0)
        {
            return items.back().val;
        }
        T t;
        ::memset(&t, 0, sizeof(t));
        return t;
    }

    inline T tail() const
    {
        if (items.size() > 0)
        {
            return items.front().val;
        }
        T t;
        ::memset(&t, 0, sizeof(t));
        return t;
    }

    inline T first() const
    {
        if (items.size() > 0)
        {
            return items.back().val;
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

    inline T operator[](size_t index) const
    {
        if (index > items.size())
        {
            return T{};
        }
        return items[index].val;
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
            std::cout << o.nanos << "," << o.val << "," << (now - o.nanos)
                      << std::endl;
        }
    }
};

} // namespace utils
} // namespace miye
