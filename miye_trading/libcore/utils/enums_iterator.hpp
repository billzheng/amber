#pragma once

#include <type_traits>

namespace miye
{
namespace utils
{

template <class EnumTraits, class LambdaT,
          std::enable_if_t<EnumTraits::iterable == true>* = nullptr>
void visit(const LambdaT& lambda)
{
    for (std::size_t i = static_cast<std::size_t>(EnumTraits::FIRST);
         i <= static_cast<std::size_t>(EnumTraits::LAST);
         ++i)
    {
        lambda(static_cast<decltype(EnumTraits::LAST)>(i));
    }
}
template <class EnumTraits, class LambdaT,
          std::enable_if_t<EnumTraits::iterable == false>* = nullptr>
void visit(const LambdaT&)
{
    static_assert(EnumTraits::iterable, "Enum not iterable");
}

} // namespace utils
} // namespace miye
