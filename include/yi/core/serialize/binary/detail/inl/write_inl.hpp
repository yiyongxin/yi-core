#pragma once

#include <algorithm>
#include <array>
#include <cstring>
#include <string>
#include <vector>
#include <cstdint>

#include "tools.h"
#include "codec.h"
#include "../detail.h"

namespace yi::serialize::binary
{

namespace detail
{

// 数值类型写入：字节序在编译期确定，if constexpr 消除运行时分支
template<bool BE, string_coding ENC, typename v_type>
inline bool write_number(BasicWriter<BE,ENC>& writer, const v_type& value)
{
  constexpr size_t v_size = sizeof(v_type);
  if (writer.offset + v_size > writer.length)
    return false;
  char* p = writer.buffer + writer.offset;
  if constexpr (BE)
    *reinterpret_cast<v_type*>(p) = value;
  else
    *reinterpret_cast<v_type*>(p) = assemble_le<v_type>(reinterpret_cast<const char*>(&value));
  writer.offset += v_size;
  return true;
}

} // namespace detail

// ── 基础类型 ──────────────────────────────────────────────────────────────

template<bool BE, string_coding ENC>
inline bool write(BasicWriter<BE,ENC>& w, const uint8_t&  v) { return detail::write_number<BE,ENC>(w, v); }
template<bool BE, string_coding ENC>
inline bool write(BasicWriter<BE,ENC>& w, const uint16_t& v) { return detail::write_number<BE,ENC>(w, v); }
template<bool BE, string_coding ENC>
inline bool write(BasicWriter<BE,ENC>& w, const uint32_t& v) { return detail::write_number<BE,ENC>(w, v); }
template<bool BE, string_coding ENC>
inline bool write(BasicWriter<BE,ENC>& w, const uint64_t& v) { return detail::write_number<BE,ENC>(w, v); }
template<bool BE, string_coding ENC>
inline bool write(BasicWriter<BE,ENC>& w, const int8_t&   v) { return detail::write_number<BE,ENC>(w, v); }
template<bool BE, string_coding ENC>
inline bool write(BasicWriter<BE,ENC>& w, const int16_t&  v) { return detail::write_number<BE,ENC>(w, v); }
template<bool BE, string_coding ENC>
inline bool write(BasicWriter<BE,ENC>& w, const int32_t&  v) { return detail::write_number<BE,ENC>(w, v); }
template<bool BE, string_coding ENC>
inline bool write(BasicWriter<BE,ENC>& w, const int64_t&  v) { return detail::write_number<BE,ENC>(w, v); }
template<bool BE, string_coding ENC>
inline bool write(BasicWriter<BE,ENC>& w, const float&    v) { return detail::write_number<BE,ENC>(w, v); }
template<bool BE, string_coding ENC>
inline bool write(BasicWriter<BE,ENC>& w, const double&   v) { return detail::write_number<BE,ENC>(w, v); }

// ── 字符串 ────────────────────────────────────────────────────────────────

template<bool BE, string_coding ENC>
inline bool write(BasicWriter<BE,ENC>& w, const std::string& v)
{
  if constexpr (ENC == string_coding::gbk)
  {
    std::string gbk;
    if (!detail::utf8_to_gbk(v, gbk))
      return false;
    const uint32_t len = static_cast<uint32_t>(gbk.size());
    if (!write(w, len))
      return false;
    if (w.offset + len > w.length)
      return false;
    std::memcpy(w.buffer + w.offset, gbk.data(), len);
    w.offset += len;
    return true;
  }
  else
  {
    const uint32_t len = static_cast<uint32_t>(v.size());
    if (!write(w, len))
      return false;
    if (w.offset + len > w.length)
      return false;
    std::memcpy(w.buffer + w.offset, v.data(), len);
    w.offset += len;
    return true;
  }
}

template<bool BE, string_coding ENC, typename length_type>
inline bool write(BasicWriter<BE,ENC>& w, const vstr<length_type>& v)
{
  if constexpr (ENC == string_coding::gbk)
  {
    std::string gbk;
    if (!detail::utf8_to_gbk(v, gbk))
      return false;
    const length_type len = static_cast<length_type>(gbk.size());
    if (!write(w, len))
      return false;
    if (w.offset + static_cast<size_t>(len) > w.length)
      return false;
    std::memcpy(w.buffer + w.offset, gbk.data(), static_cast<size_t>(len));
    w.offset += static_cast<size_t>(len);
    return true;
  }
  else
  {
    const length_type len = static_cast<length_type>(v.size());
    if (!write(w, len))
      return false;
    if (w.offset + static_cast<size_t>(len) > w.length)
      return false;
    std::memcpy(w.buffer + w.offset, v.data(), static_cast<size_t>(len));
    w.offset += static_cast<size_t>(len);
    return true;
  }
}

template<bool BE, string_coding ENC, size_t N>
inline bool write(BasicWriter<BE,ENC>& w, const fstr<N>& v)
{
  if (w.offset + N > w.length)
    return false;
  if constexpr (ENC == string_coding::gbk)
  {
    std::string gbk;
    if (!detail::utf8_to_gbk(v, gbk))
      return false;
    const size_t copy_len = std::min(gbk.size(), N);
    std::memcpy(w.buffer + w.offset, gbk.data(), copy_len);
    if (copy_len < N)
      std::memset(w.buffer + w.offset + copy_len, 0, N - copy_len);
  }
  else
  {
    const size_t copy_len = std::min(v.size(), N);
    std::memcpy(w.buffer + w.offset, v.data(), copy_len);
    if (copy_len < N)
      std::memset(w.buffer + w.offset + copy_len, 0, N - copy_len);
  }
  w.offset += N;
  return true;
}

// ── 容器 ──────────────────────────────────────────────────────────────────

template<bool BE, string_coding ENC, typename T>
inline bool write(BasicWriter<BE,ENC>& w, const std::vector<T>& v)
{
  const uint32_t cnt = static_cast<uint32_t>(v.size());
  if (!write(w, cnt))
    return false;
  for (const auto& item : v)
  {
    if (!write(w, item))
      return false;
  }
  return true;
}

template<bool BE, string_coding ENC, typename T, typename length_type>
inline bool write(BasicWriter<BE,ENC>& w, const vec<T,length_type>& v)
{
  const length_type cnt = static_cast<length_type>(v.size());
  if (!write(w, cnt))
    return false;
  for (const auto& item : v)
  {
    if (!write(w, item))
      return false;
  }
  return true;
}

template<bool BE, string_coding ENC, typename T, size_t N>
inline bool write(BasicWriter<BE,ENC>& w, const std::array<T,N>& v)
{
  for (const auto& item : v)
  {
    if (!write(w, item))
      return false;
  }
  return true;
}

} // namespace yi::serialize::binary
