#pragma once
#include <cstdint>
#include <stdint.h>
#include <tuple>

#include "../ftx_fix/fix_number_utils.hpp"

namespace miye::trading::fix
{

using FixId_t = uint32_t;

template <uint32_t ID>
class FieldPtr
{
  public:
    using len_t = uint32_t;

  public:
    constexpr FieldPtr() = default;
    constexpr FieldPtr(const char* begin, uint32_t len) noexcept : begin_(begin), len_(len) {}

    constexpr FieldPtr(const std::tuple<FixId_t, const char*, uint32_t>& def) noexcept
        : begin_(std::get<1>(def)), len_(std::get<2>(def))
    {
    }

    void setPointers(const std::tuple<FixId_t, const char*, uint32_t>& def) noexcept
    {
        begin_ = std::get<1>(def);
        len_   = std::get<2>(def);
    }

    template <typename T>
    double toFloatingPoint() const
    {
        return fix::toFloatingPoint<T>(begin_, (size_t)len_);
    }

    template <typename T>
    T toInteger() const
    {
        return toInteger<T>(std::is_unsigned<T>());
    }

    template <typename T>
    T toEnum() const
    {
        return toEnum<T>(
            typename std::integral_constant<bool,
                                            std::is_same<char, typename std::underlying_type<T>::type>::value>::type{});
    }

    std::string toString() const { return isNull() ? std::string() : std::string(begin_, len_); }

    void reset() noexcept
    {
        begin_ = nullptr;
        len_   = 0;
    }

    constexpr static FixId_t Id() noexcept { return ID; }
    void setBegin(const char* begin) noexcept { begin_ = begin; }
    const char* getBegin() const noexcept { return begin_; }
    uint32_t getLen() const noexcept { return len_; }
    void setLen(uint32_t len) { len_ = len; }

    bool isNull() const noexcept { return begin_ == nullptr || len_ == 0; }

  private:
    // for signed integer
    template <typename T>
    T toInteger(std::false_type) const
    {
        assert(len_ > 0);
        return fix::toInteger<T>(begin_, (size_t)len_);
    }

    // for unsigned integer
    template <typename T>
    T toInteger(std::true_type) const
    {
        assert(len_ > 0);
        return fix::toUInteger<T>(begin_, (size_t)len_);
    }

    // for underlying is int enum
    template <typename T>
    T toEnum(std::false_type) const
    {
        return T(toInteger<std::underlying_type_t<T>>());
    }

    // for underlying is char enum
    template <typename T>
    T toEnum(std::true_type) const
    {
        assert(len_ > 0);
        return T(begin_[0]);
    }

  private:
    const char* begin_{nullptr};
    uint32_t len_{};
};

} // namespace miye::trading::fix
