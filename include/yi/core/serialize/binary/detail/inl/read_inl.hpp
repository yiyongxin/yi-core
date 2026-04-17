#pragma once

#include <array>
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

// 数值类型读取：字节序在编译期确定，if constexpr 消除运行时分支
template<bool BE, string_coding ENC, typename v_type>
inline bool read_number(BasicReader<BE,ENC>& reader, v_type& value)
{
  constexpr size_t v_size = sizeof(v_type);
  if (reader.offset + v_size > reader.length)
    return false;
  const char* p = reader.buffer + reader.offset;
  if constexpr (BE)
    value = assemble_be<v_type>(p);
  else
    value = assemble_le<v_type>(p);
  reader.offset += v_size;
  return true;
}

} // namespace detail

// ── 基础类型 ──────────────────────────────────────────────────────────────

template<bool BE, string_coding ENC>
inline bool read(BasicReader<BE,ENC>& r, uint8_t&  v) { return detail::read_number<BE,ENC>(r, v); }
template<bool BE, string_coding ENC>
inline bool read(BasicReader<BE,ENC>& r, uint16_t& v) { return detail::read_number<BE,ENC>(r, v); }
template<bool BE, string_coding ENC>
inline bool read(BasicReader<BE,ENC>& r, uint32_t& v) { return detail::read_number<BE,ENC>(r, v); }
template<bool BE, string_coding ENC>
inline bool read(BasicReader<BE,ENC>& r, uint64_t& v) { return detail::read_number<BE,ENC>(r, v); }
template<bool BE, string_coding ENC>
inline bool read(BasicReader<BE,ENC>& r, int8_t&   v) { return detail::read_number<BE,ENC>(r, v); }
template<bool BE, string_coding ENC>
inline bool read(BasicReader<BE,ENC>& r, int16_t&  v) { return detail::read_number<BE,ENC>(r, v); }
template<bool BE, string_coding ENC>
inline bool read(BasicReader<BE,ENC>& r, int32_t&  v) { return detail::read_number<BE,ENC>(r, v); }
template<bool BE, string_coding ENC>
inline bool read(BasicReader<BE,ENC>& r, int64_t&  v) { return detail::read_number<BE,ENC>(r, v); }
template<bool BE, string_coding ENC>
inline bool read(BasicReader<BE,ENC>& r, float&    v) { return detail::read_number<BE,ENC>(r, v); }
template<bool BE, string_coding ENC>
inline bool read(BasicReader<BE,ENC>& r, double&   v) { return detail::read_number<BE,ENC>(r, v); }

// ── 字符串 ────────────────────────────────────────────────────────────────

template<bool BE, string_coding ENC>
inline bool read(BasicReader<BE,ENC>& r, std::string& v)
{
  uint32_t len = 0;
  if (!read(r, len))
    return false;
  if (r.offset + len > r.length)
    return false;
  if constexpr (ENC == string_coding::gbk)
  {
    if (!detail::gbk_to_utf8(r.buffer + r.offset, len, v))
      v.assign(r.buffer + r.offset, len);
  }
  else
  {
    v.assign(r.buffer + r.offset, len);
  }
  r.offset += len;
  return true;
}

template<bool BE, string_coding ENC, typename length_type>
inline bool read(BasicReader<BE,ENC>& r, vstr<length_type>& v)
{
  if (!read(r, v._yi_core_serialize_binary_variable_length))
    return false;
  const size_t len = static_cast<size_t>(v._yi_core_serialize_binary_variable_length);
  if (r.offset + len > r.length)
    return false;
  if constexpr (ENC == string_coding::gbk)
  {
    std::string utf8_str;
    if (!detail::gbk_to_utf8(r.buffer + r.offset, len, utf8_str))
      v.assign(r.buffer + r.offset, len);
    else
      v.assign(utf8_str);
  }
  else
  {
    v.assign(r.buffer + r.offset, len);
  }
  r.offset += len;
  return true;
}

template<bool BE, string_coding ENC, size_t N>
inline bool read(BasicReader<BE,ENC>& r, fstr<N>& v)
{
  if (r.offset + N > r.length)
    return false;
  if constexpr (ENC == string_coding::gbk)
  {
    const char* p = r.buffer + r.offset;
    size_t raw_len = N;
    for (size_t i = 0; i < N; ++i)
    {
      if (p[i] == '\0')
      {
        raw_len = i;
        break;
      }
    }
    if (!detail::gbk_to_utf8(p, raw_len, v))
      v.assign(p, raw_len);
  }
  else
  {
    v.assign(r.buffer + r.offset, N);
    const size_t end = v.find('\0');
    if (end != std::string::npos)
      v.resize(end);
  }
  r.offset += N;
  return true;
}

// ── 容器 ──────────────────────────────────────────────────────────────────

template<bool BE, string_coding ENC, typename T>
inline bool read(BasicReader<BE,ENC>& r, std::vector<T>& v)
{
  uint32_t cnt = 0;
  if (!read(r, cnt))
    return false;
  v.clear();
  v.reserve(cnt);
  for (uint32_t i = 0; i < cnt; ++i)
  {
    T item{};
    if (!read(r, item))
      return false;
    v.push_back(std::move(item));
  }
  return true;
}

template<bool BE, string_coding ENC, typename T, typename length_type>
inline bool read(BasicReader<BE,ENC>& r, vec<T,length_type>& v)
{
  length_type cnt{};
  if (!read(r, cnt))
    return false;
  v.clear();
  v.reserve(static_cast<size_t>(cnt));
  for (length_type i = 0; i < cnt; ++i)
  {
    T item{};
    if (!read(r, item))
      return false;
    v.push_back(std::move(item));
  }
  return true;
}

template<bool BE, string_coding ENC, typename T, size_t N>
inline bool read(BasicReader<BE,ENC>& r, std::array<T,N>& v)
{
  for (auto& item : v)
  {
    if (!read(r, item))
      return false;
  }
  return true;
}

} // namespace yi::serialize::binary
