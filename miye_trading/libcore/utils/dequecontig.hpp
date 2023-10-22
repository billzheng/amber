/*
 * deque_contig.hpp
 * std::deque like data structure using contiguous
 * memory for storage
 */

#pragma once
#include "libcore/utils/vector.hpp"

#if 0
#define QDEBUG(...) std::cerr << __func__ << "(): " << __VA_ARGS__
#define QDEBUG_CONT(...) std::cerr << __VA_ARGS__
#else
#define QDEBUG(...)
#define QDEBUG_CONT(...)
#endif

// end_offset_ give the equivalent place to the end() iterator.
// i.e. one past the last item in the queue
// start_offset_ is the index of the first item in the queue;

namespace miye
{
namespace fundamentals
{

static const int initial_capacity = 8;
template <typename T>
class dequecontig
{
  public:
    explicit dequecontig()
        : start_offset_(initial_capacity / 2),
          end_offset_(initial_capacity / 2), last_removed_(nullptr)
    {
        data_.resize(initial_capacity);
    }

    explicit dequecontig(dequecontig const& rhs)
    {
        QDEBUG(__PRETTY_FUNCTION__ << std::endl);
        QDEBUG("rhs= " << rhs << std::endl);
        auto cap = rhs.capacity();
        start_offset_ = (cap / 2);
        data_.resize(cap);
        end_offset_ = (cap / 2);
        auto sz = rhs.places_used();
        for (int32_t i = 0; i != sz; ++i)
        {
            T item = rhs[i];
            this->push_back(item);
        }
        QDEBUG("this= " << *this << std::endl);
    }
    dequecontig(dequecontig const&& rhs) = delete;
    dequecontig& operator=(dequecontig const&& rhs) = delete;

    dequecontig& operator=(dequecontig const& rhs)
    {
        QDEBUG(__PRETTY_FUNCTION__ << std::endl);
        QDEBUG("rhs= " << rhs << std::endl);
        auto cap = rhs.capacity();
        data_.resize(cap);
        start_offset_ = (cap / 2);
        end_offset_ = (cap / 2);
        auto sz = rhs.places_used();
        for (int32_t i = 0; i != sz; ++i)
        {
            T item = rhs[i];
            this->push_back(item);
        }
        QDEBUG("this= " << *this << std::endl);
        return *this;
    }

    T const& operator[](int32_t idx) const
    {
        auto real_idx = idx + start_offset_;
        auto cap = capacity();
        // real_idx = real_idx ^ ((real_idx ^ (real_idx - cap)) & (!(real_idx <
        // cap)));
        if (real_idx >= cap)
        {
            real_idx -= cap;
        }
        ASSERT_MSG(real_idx >= 0, DUMP(real_idx));
        ASSERT_MSG(real_idx < cap, DUMP(real_idx) << DUMP(cap));
        ASSERT_MSG(idx < cap, DUMP(idx) << DUMP(cap));
        ASSERT_MSG(idx < places_used(), DUMP(idx) << DUMP(places_used()));
        ASSERT_MSG(places_used() > 0, DUMP(idx) << DUMP(places_used()));
        ASSERT(&data_[0] <= &data_[real_idx]);
        ASSERT((&data_[cap - 1] + 1) > &data_[real_idx]);
        return data_[real_idx];
    }

    // non-const calls the const and casts back to avoid code duplication
    // ecpp3ed item3 p24
    T& operator[](int32_t idx)
    {
        return const_cast<T&>(static_cast<const dequecontig&>(*this)[idx]);
    }

    T& push_back(T const& item)
    {
        check_capacity();
        auto idx = end_offset_;
        ++end_offset_;
        check_wrap_end();
        data_[idx] = item;
        return data_[idx];
    }

    T pop_back()
    {
        if (start_offset_ == end_offset_)
        {
            return *last_removed_;
        }
        --end_offset_;
        if (end_offset_ == -1)
        {
            end_offset_ = capacity() - 1;
        }
        last_removed_ = &data_[end_offset_];
        return *last_removed_;
    }
    T& push_front(T const& item)
    {
        check_capacity();
        --start_offset_;
        check_wrap_start();
        data_[start_offset_] = item;
        return data_[start_offset_];
    }

    T pop_front()
    {
        if (start_offset_ == end_offset_)
        {
            return *last_removed_;
        }
        last_removed_ = &data_[start_offset_];
        ++start_offset_;
        check_wrap_end();
        return *last_removed_;
    }

    // these won't work with STL algorithms
    // TODO iterator class with wrap aware member functions
    // so that repeated ++it on begin will eventually wrap and
    // get to end when the end has wrapped around through the
    // start of the contiguous memory block
  private:
    T* begin()
    {
        return &data_[start_offset_];
    }
    const T* begin() const
    {
        return &data_[start_offset_];
    }
    T* end()
    {
        return &data_[end_offset_];
    }
    const T* end() const
    {
        return &data_[end_offset_];
    }

  public:
    T& back()
    {
        return (*this)[this->size() - 1];
    }
    const T& back() const
    {
        return (*this)[this->size() - 1];
    }
    T& front()
    {
        return *begin();
    }
    const T& front() const
    {
        return *begin();
    }

    int32_t capacity() const
    {
        QDEBUG(DUMP(start_offset_)
               << DUMP(end_offset_) << DUMP(data_.size()) << std::endl);
        return data_.size();
    }
    size_t size() const
    {
        return places_used();
    }
    bool empty() const
    {
        return size() == 0;
    }
    int32_t places_used() const
    {
        QDEBUG(DUMP(start_offset_) << DUMP(end_offset_) << DUMP(data_.size()));
        if (end_offset_ >= start_offset_)
        {
            QDEBUG_CONT(DUMP(end_offset_ - start_offset_) << std::endl);
            return end_offset_ - start_offset_;
        }
        // we've wrapped so the end is before the start in memory
        // from start of used region to end of array
        // add from offset zero to end offset
        int32_t cap = capacity();
        int32_t used = (cap - start_offset_) + end_offset_;
        INVARIANT_MSG(used < cap, DUMP(used) << DUMP(cap));
        QDEBUG_CONT(DUMP(used) << std::endl);
        return used;
    }

    T& last_removed()
    {
        return *last_removed_;
    }

    void clear()
    {
        start_offset_ = capacity() / 2;
        end_offset_ = start_offset_;
        ASSERT(size() == 0);
    }
    void reserve(int32_t size)
    {
        auto orig_cap = capacity();
        // round up the request to a power of 2
        auto rounded_up_sz = orig_cap * 2;
        if (size > orig_cap)
        {
            while (rounded_up_sz < size)
            {
                rounded_up_sz *= 2;
            }
            data_.resize(rounded_up_sz);
            unwrap_into_larger(orig_cap);
        }
    }

  private:
    void check_wrap_end()
    {
        QDEBUG(DUMP(start_offset_)
               << DUMP(end_offset_) << DUMP(data_.size()) << std::endl);
        if (end_offset_ == capacity())
        {
            end_offset_ = 0;
        }
        if (start_offset_ == capacity())
        {
            start_offset_ = 0;
        }
    }
    void check_wrap_start()
    {
        QDEBUG(DUMP(start_offset_)
               << DUMP(end_offset_) << DUMP(data_.size()) << std::endl);
        if (start_offset_ == -1)
        {
            start_offset_ = capacity() - 1;
        }
        if (end_offset_ == -1)
        {
            end_offset_ = capacity() - 1;
        }
    }
    void check_capacity()
    {
        QDEBUG(DUMP(start_offset_)
               << DUMP(end_offset_) << DUMP(data_.size()) << std::endl);
        auto used = places_used();
        auto cap = capacity();
        if (cap - used <= 2)
        {
            QDEBUG(DUMP(cap) << DUMP(used) << DUMP((cap - used))
                             << "resizing to " << cap * 2 << std::endl);
            data_.resize(cap * 2);
            unwrap_into_larger(cap);
        }
    }
    void unwrap_into_larger(int32_t previous_cap)
    {
        if (end_offset_ < start_offset_)
        {
            QDEBUG("unrwapping after resize\n");
            for (int i = 0; i != end_offset_; ++i)
            {
                data_[previous_cap + i] = data_[i];
            }
            end_offset_ = previous_cap + end_offset_;
        }
    }

  public:
    int32_t start_offset_;
    int32_t end_offset_;
    T* last_removed_{nullptr};
    fundamentals::vector<T> data_;
};

template <typename OS, typename T>
OS& operator<<(OS& os, dequecontig<T> const& d)
{
    os << "dequecontig { capacity = " << d.capacity() << ",\n";
    os << "places_used() (aka size()) = " << d.size() << ",\n";
    os << "[\n";
    for (int i = 0; i != d.places_used(); ++i)
    {
        os << "[" << i << "] = " << d[i] << "\n";
    }
    os << "]\n";

    os << "underlying vect: \n";

    os << "[\n";
    for (int i = 0; i != d.capacity(); ++i)
    {
        os << "[" << i << "] = " << d.data_[i];
        if (i == d.start_offset_)
            os << " <= start offset";
        if (i == d.end_offset_)
            os << " <= end offset";
        os << "\n";
    }
    os << "]\n";

    return os;
}

} // namespace fundamentals
} // namespace miye

#undef QDEBUG
#undef QDEBUG_CONT
