#pragma once
#include <cassert>
#include "../ftx_ftx/field_ptr.hpp"

namespace miye::trading::fix
{

class FixIstream
{
  public:
    constexpr static const uint8_t separator = 0x01;

  public:
    constexpr FixIstream() = default;

    void setBuffer(const char* begin, size_t len)
    {
        begin_ = begin;
        end_   = begin + len;
        curr_  = begin;
    }

    std::tuple<FixId_t, const char*, uint32_t> getCurrent() noexcept
    {
        FixId_t fieldId{0};

        // sample fix field pair: 34=123456
        while (*curr_ != '=')
        {
            fieldId *= 10;
            fieldId += *curr_ - '0';
            ++curr_;
        }

        assert(curr_ < end_);
        assert(fieldId > 0);

        ++curr_; // move pointer over '='

        const char* val_begin{curr_};
        // look for end of fix value
        while (*curr_ != separator)
        {
            ++curr_;
        }

        ++curr_; // move pointer over field separator
        return std::make_tuple(fieldId, val_begin, curr_ - val_begin - 1);
    }

    bool hasNext() const noexcept { return curr_ < end_; }

    void reset() noexcept
    {
        begin_ = nullptr;
        end_   = nullptr;
        curr_  = nullptr;
    }

  private:
    const char* begin_{nullptr};
    const char* end_{nullptr};
    const char* curr_{nullptr};
};

} // namespace miye::trading::fix
