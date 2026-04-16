#pragma once

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>
#include <cstdint>

#include "tools.h"
#include "../detail.h"

namespace yi::serialize::binary
{

namespace detail
{
// 数值类型写入
template<typename v_type>
inline bool write_number(Writer& writer, const v_type& value)
{
  constexpr size_t v_size = sizeof(v_type);
  if (writer.offset + v_size > writer.length)
    return false;
  char* point = writer.buffer + writer.offset;
  if (writer.is_big_endian)
    *reinterpret_cast<v_type*>(point) = value;
  else
    *reinterpret_cast<v_type*>(point) = assemble_le<v_type>(reinterpret_cast<const char*>(&value));
  writer.offset += v_size;
  return true;
}
}
//------------------------------基础类型支持---------------------------------

inline bool write(Writer& writer, const uint8_t&  value) { return detail::write_number<uint8_t> (writer, value); }
inline bool write(Writer& writer, const uint16_t& value) { return detail::write_number<uint16_t>(writer, value); }
inline bool write(Writer& writer, const uint32_t& value) { return detail::write_number<uint32_t>(writer, value); }
inline bool write(Writer& writer, const uint64_t& value) { return detail::write_number<uint64_t>(writer, value); }
inline bool write(Writer& writer, const int8_t&   value) { return detail::write_number<int8_t>  (writer, value); }
inline bool write(Writer& writer, const int16_t&  value) { return detail::write_number<int16_t> (writer, value); }
inline bool write(Writer& writer, const int32_t&  value) { return detail::write_number<int32_t> (writer, value); }
inline bool write(Writer& writer, const int64_t&  value) { return detail::write_number<int64_t> (writer, value); }
inline bool write(Writer& writer, const float&    value) { return detail::write_number<float>   (writer, value); }
inline bool write(Writer& writer, const double&   value) { return detail::write_number<double>  (writer, value); }

//-------------------------------字符串支持-------------------------------

inline bool write(Writer& writer, const std::string& value)
{
  const uint32_t len = static_cast<uint32_t>(value.size());
  if (!write(writer, len)) return false;
  if (writer.offset + len > writer.length) return false;
  std::memcpy(writer.buffer + writer.offset, value.data(), len);
  writer.offset += len;
  return true;
}

template <typename length_type>
inline bool write(Writer& writer, const vstr<length_type>& value)
{
  const length_type len = static_cast<length_type>(value.data.size());
  if (!write(writer, len)) return false;
  if (writer.offset + len > writer.length) return false;
  std::memcpy(writer.buffer + writer.offset, value.data.data(), len);
  writer.offset += len;
  return true;
}

template <size_t N>
inline bool write(Writer& writer, const fstr<N>& value)
{
  if (writer.offset + N > writer.length) return false;
  const size_t copy_len = std::min(value.data.size(), N);
  std::memcpy(writer.buffer + writer.offset, value.data.data(), copy_len);
  if (copy_len < N)
    std::memset(writer.buffer + writer.offset + copy_len, 0, N - copy_len);
  writer.offset += N;
  return true;
}

//-------------------------------容器类型支持-------------------------------

template<typename T>
inline bool write(Writer& writer, const std::vector<T>& value)
{
  const uint32_t cnt = static_cast<uint32_t>(value.size());
  if (!write(writer, cnt)) return false;
  for (const auto& item : value)
    if (!write(writer, item)) return false;
  return true;
}

} // namespace yi::serialize::binary
