#pragma once

#include <functional>

namespace miye::risk
{

inline void hash_combine(std::size_t& seed) {}

template <typename T, typename... Args>
inline void hash_combine(std::size_t seed, const T& v, Args... args)
{
    std::hash<T> hasher;
    // boost::hash_combined implementation
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    hash_combine(seed, args...);
}

} // namespace miye::risk
