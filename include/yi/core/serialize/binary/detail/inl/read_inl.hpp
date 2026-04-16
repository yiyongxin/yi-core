#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "tools.h"
#include "../detail.h"

namespace yi::serialize::binary
{

namespace detail
{
// 数值类型读取
template<typename v_type>
inline bool read_number(Reader& reader, v_type& value)
{
  constexpr size_t v_size = sizeof(v_type);
  if (reader.offset + v_size > reader.length)
    return false;
  const char* point = reader.buffer + reader.offset;
  if (reader.is_big_endian)
    value = *reinterpret_cast<const v_type*>(point);
  else
    value = assemble_le<v_type>(point);
  reader.offset += v_size;
  return true;
}
}
//------------------------------基础类型支持---------------------------------

inline bool read(Reader& reader, uint8_t&  value) { return detail::read_number<uint8_t> (reader, value); }
inline bool read(Reader& reader, uint16_t& value) { return detail::read_number<uint16_t>(reader, value); }
inline bool read(Reader& reader, uint32_t& value) { return detail::read_number<uint32_t>(reader, value); }
inline bool read(Reader& reader, uint64_t& value) { return detail::read_number<uint64_t>(reader, value); }
inline bool read(Reader& reader, int8_t&   value) { return detail::read_number<int8_t>  (reader, value); }
inline bool read(Reader& reader, int16_t&  value) { return detail::read_number<int16_t> (reader, value); }
inline bool read(Reader& reader, int32_t&  value) { return detail::read_number<int32_t> (reader, value); }
inline bool read(Reader& reader, int64_t&  value) { return detail::read_number<int64_t> (reader, value); }
inline bool read(Reader& reader, float&    value) { return detail::read_number<float>   (reader, value); }
inline bool read(Reader& reader, double&   value) { return detail::read_number<double>  (reader, value); }

//-------------------------------字符串支持-------------------------------

inline bool read(Reader& reader, std::string& value)
{
  uint32_t len = 0;
  if (!read(reader, len))
    return false;
  if (reader.offset + len > reader.length)
    return false;
  value.assign(reader.buffer + reader.offset, len);
  reader.offset += len;
  return true;
}

template <typename length_type>
inline bool read(Reader& reader, vstr<length_type>& value)
{
  if (!read(reader, value.length))
    return false;
  if (reader.offset + value.length > reader.length)
    return false;
  value.data.assign(reader.buffer + reader.offset, value.length);
  reader.offset += value.length;
  return true;
}

template <size_t N>
inline bool read(Reader& reader, fstr<N>& value)
{
  if (reader.offset + N > reader.length)
    return false;
  value.data.assign(reader.buffer + reader.offset, N);
  // 去除末尾 null 填充
  const size_t end = value.data.find('\0');
  if (end != std::string::npos)
    value.data.resize(end);
  reader.offset += N;
  return true;
}

//-------------------------------容器类型支持-------------------------------

template<typename T>
inline bool read(Reader& reader, std::vector<T>& value)
{
  uint32_t cnt = 0;
  if (!read(reader, cnt))
    return false;
  value.clear();
  value.reserve(cnt);
  for (uint32_t i = 0; i < cnt; ++i) {
    T item{};
    if (!read(reader, item))
      return false;
    value.push_back(std::move(item));
  }
  return true;
}

} // namespace yi::serialize::binary
