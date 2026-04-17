#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <vector>
#include <cstdint>

#include "../detail.h"

namespace yi::serialize::binary
{

// ── 基础类型 ──────────────────────────────────────────────────────────────

inline constexpr size_t size(const uint8_t&)  { return 1; }
inline constexpr size_t size(const uint16_t&) { return 2; }
inline constexpr size_t size(const uint32_t&) { return 4; }
inline constexpr size_t size(const uint64_t&) { return 8; }
inline constexpr size_t size(const int8_t&)   { return 1; }
inline constexpr size_t size(const int16_t&)  { return 2; }
inline constexpr size_t size(const int32_t&)  { return 4; }
inline constexpr size_t size(const int64_t&)  { return 8; }
inline constexpr size_t size(const float&)    { return 4; }
inline constexpr size_t size(const double&)   { return 8; }

// ── 字符串 ────────────────────────────────────────────────────────────────

inline size_t size(const std::string& value) { return 4 + value.size(); }

template <typename length_type>
inline size_t size(const vstr<length_type>& value) { return size(length_type()) + value.size(); }

template <size_t N>
inline constexpr size_t size(const fstr<N>&) { return N; }

// ── 容器 ──────────────────────────────────────────────────────────────────

template<typename T>
inline size_t size(const std::vector<T>& value)
{
  size_t total = sizeof(uint32_t);
  for (const auto& item : value)
    total += size(item);
  return total;
}

template <typename T, typename length_type>
inline size_t size(const vec<T,length_type>& value)
{
  size_t total = size(length_type());
  for (const auto& item : value)
    total += size(item);
  return total;
}

template<typename T, size_t N>
inline size_t size(const std::array<T,N>& value)
{
  size_t total = 0;
  for (const auto& item : value)
    total += size(item);
  return total;
}

} // namespace yi::serialize::binary
