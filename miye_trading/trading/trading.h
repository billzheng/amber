#pragma once
#include "libcore/math/math_util.h"
#include "libcore/types/types.hpp"
#include "libcore/utils/queue.hpp"
#include <cassert>

namespace miye
{
namespace trading
{

struct Signal
{
    struct SignalFlags
    {
        static const uint32_t buyOnly = 1;
        static const uint32_t sellOnly = 2;
    };

    void set(double v, uint16_t f)
    {
        this->value = v;
        this->flags = f;
    }

    void setValue(double v)
    {
        this->value = v;
    }
    double getValue() const noexcept
    {
        return this->value;
    }

    void clip(double minValue, double maxValue)
    {
        assert(math::lessEqual(minValue, maxValue));
        if (value < minValue)
        {
            value = minValue;
        }
        if (value > maxValue)
        {
            value = maxValue;
        }
    }
    void setBuyOnly()
    {
        flags |= SignalFlags::buyOnly;
    }

    void setSellOnly()
    {
        flags |= SignalFlags::sellOnly;
    }

    bool allowdToBuy() const noexcept
    {
        return !(flags & SignalFlags::sellOnly);
    }

    bool allowdToSell() const noexcept
    {
        return !(flags & SignalFlags::buyOnly);
    }

    Signal& operator+=(const Signal& rhs)
    {
        this->value += rhs.value;
        this->flags |= rhs.flags;
        return *this;
    }
    Signal& operator+(const Signal& rhs)
    {
        this->value += rhs.value;
        this->flags |= rhs.flags;
        return *this;
    }
    Signal& operator=(const Signal& rhs)
    {
        this->value = rhs.value;
        this->flags = rhs.flags;
        return *this;
    }

    double value{0.0};
    uint16_t flags{0};
};

struct Alpha
{
    Alpha() = default;
    Alpha(double lowBound, double highBound)
        : lowBound(lowBound), highBound(highBound)
    {
    }

    void setRawAlpha(double v)
    {
        this->rawAlpha = v;
    }
    void applyWeight(double weight)
    {
        return cappedSignal.setValue(rawAlpha * weight);
    }

    bool eval(double weight)
    {
        if (std::isnan(rawAlpha) || std::isinf(rawAlpha))
        {
            return false;
        }

        clip();
        applyWeight(weight);
        return true;
    }

    void clip()
    {
        if (rawAlpha < lowBound)
        {
            rawAlpha = lowBound;
        }
        if (rawAlpha > highBound)
        {
            rawAlpha = highBound;
        }
    }

    Signal cappedSignal{0.0};
    double rawAlpha{0.0};
    double lowBound{-1};
    double highBound{1};
};

template <typename OS>
inline OS& operator<<(OS& os, const Alpha& o)
{
    os << "(rawAlpha:" << o.rawAlpha
       << ",cappedSignal:" << o.cappedSignal.getValue() << ")";
    return os;
}

} // namespace trading
} // namespace miye
