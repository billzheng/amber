#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstring>
#include <stdint.h>
#include <string>

#include "../ftx_fix/double_to_string.hpp"
#include "../ftx_fix/number_to_string.hpp"
#include "../ftx_fix/string_const.hpp"
#include "../ftx_ftx/fix_number_utils.hpp"

namespace miye::trading::fix
{

constexpr static const char FixSeparator('\001');

/*
 * Fix Field class prebuilds Prefix(include FixTag and '=') into beginning of buffer
 * also calculate length and checksum of Prefix at compile time
 */
template <uint32_t FixId, size_t Len>
class Field
{
  public:
    constexpr static const uint32_t PrefixLen = sizeof(acutus::string_from<unsigned, FixId>::value);

    // sample value:34=
    constexpr static auto const Prefix             = acutus::string_from<unsigned, FixId>::value;
    constexpr static uint32_t const PrefixCheckSum = fix::calcCheckSum(acutus::string_from<unsigned, FixId>::value);
    constexpr static const size_t BufferLen        = Len + PrefixLen + 1;

    Field() noexcept { init(); }

    void set(const char* p, size_t length) noexcept
    {
        assert(length <= Len);

        auto minLen = std::min(BufferLen - PrefixLen, length);
        std::memcpy(val_.data() + PrefixLen, p, minLen);
        val_[PrefixLen + minLen] = FixSeparator;
        minLen++;

        checkSum_ = PrefixCheckSum + fix::calcCheckSum(val_.data() + PrefixLen, minLen);
        fieldLen_ = PrefixLen + minLen;
    }

    void set(const std::string& v) noexcept { set(v.data(), v.length()); }

    void set(const acutus::string_const& v) noexcept { set(v.begin(), v.size()); }

    void set(char v) noexcept
    {
        fieldLen_       = PrefixLen + 1;
        val_[PrefixLen] = v;
        val_[fieldLen_] = FixSeparator;
        checkSum_       = PrefixCheckSum + v + FixSeparator;
        fieldLen_++;
    }

    template <typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
    void set(T v) noexcept
    {
        auto len = fix::i64toa(v, val_.data() + PrefixLen);
        assert(len <= Len);
        val_[PrefixLen + len] = FixSeparator;
        len++;

        checkSum_ = PrefixCheckSum + fix::calcCheckSum(val_.data() + PrefixLen, len);
        fieldLen_ = PrefixLen + len;
    }

    void set(double v, uint32_t precision) noexcept
    {
        auto len = acutus::dtoa(v, val_.data() + PrefixLen, precision);
        assert(len <= Len);
        val_[PrefixLen + len] = FixSeparator;
        len++;

        checkSum_ = PrefixCheckSum + fix::calcCheckSum(val_.data() + PrefixLen, len);
        fieldLen_ = PrefixLen + len;
    }
    void reset() noexcept { init(); }

    void init()
    {
        std::fill(std::begin(val_), std::end(val_), FixSeparator);
        std::memcpy(val_.data(), Prefix, PrefixLen);
        fieldLen_ = 0;
        checkSum_ = 0;
    }

    std::string toString() const { return std::string(val_.data(), fieldLen_); }
    const std::array<char, BufferLen>& getValue() const noexcept { return val_; }

    const char* begin() const noexcept { return val_.data(); }
    uint32_t len() const noexcept { return fieldLen_; }

    uint32_t id() const noexcept { return FixId; }
    uint32_t getCheckSum() const noexcept { return checkSum_; }
    bool isSet() const noexcept { return checkSum_ > 0 && fieldLen_ > 0; }
    constexpr size_t getMaxValueLen() const noexcept { return Len; }

  private:
    std::array<char, BufferLen> val_;
    uint32_t fieldLen_{};
    uint32_t checkSum_{};
};

struct CheckSumField
{
    CheckSumField() { reset(); }

    void reset() { val_ = {'1', '0', '=', '0', '0', '0', '\x01'}; }
    void set(uint16_t v) { to_cksum_str(v, val_.data() + 3); }

    /*
     * checksum needs to be exact 3 bytes
     */
    inline void to_cksum_str(uint16_t v, char* buffer, char pad = '0')
    {
        assert(v < 1000);

        size_t offset = 0;
        if (v < 100)
        {
            offset = 1;
            if (v < 10)
            {
                offset = 2;
            }
        }

        for (size_t i = 0; i < offset; ++i)
        {
            buffer[i] = pad;
        }

        fix::u32toa(v, buffer + offset);
    }

    size_t render(char* p)
    {
        std::memcpy(p, val_.begin(), val_.size());
        return val_.size();
    }

    uint32_t id() const noexcept { return 10; }
    std::array<char, 7> val_;
};

} // namespace miye::trading::fix
