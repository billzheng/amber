#pragma once

template<typename E>
constexpr inline auto toUnderlyingType(E e) -> typename std::underlying_type<E>::type
{
   return static_cast<typename std::underlying_type<E>::type>(e);
}
