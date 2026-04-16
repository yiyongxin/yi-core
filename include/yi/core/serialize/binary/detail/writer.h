#pragma once

#include <array>
#include <string>
#include <vector>
#include <cstdint>

#include "detail.h"

namespace yi::serialize::binary
{
//------------------------------基础类型支持---------------------------------

bool write(Writer& writer, const uint8_t&  value);
bool write(Writer& writer, const uint16_t& value);
bool write(Writer& writer, const uint32_t& value);
bool write(Writer& writer, const uint64_t& value);
bool write(Writer& writer, const int8_t&   value);
bool write(Writer& writer, const int16_t&  value);
bool write(Writer& writer, const int32_t&  value);
bool write(Writer& writer, const int64_t&  value);
bool write(Writer& writer, const float&    value);
bool write(Writer& writer, const double&   value);
//-------------------------------字符串支持-------------------------------

// 默认4字节长度前缀的变长字符串
bool write(Writer& writer, const std::string& value);
// 自定义长度前缀的变长字符串
template <typename length_type>
bool write(Writer& writer, const vstr<length_type>& value);
// 定长字符串（不足N字节补零，超出N字节截断）
template <size_t N>
bool write(Writer& writer, const fstr<N>& value);
//-------------------------------容器类型支持-------------------------------

//读取变长数组 默认4字节可变长数组
template<typename T>
bool write(Writer& writer, const std::vector<T>& value);
//读取变长数组
template <typename T, typename length_type>
bool write(Writer& writer, const vec<T,length_type>& value);
//读取定长数组
template<typename T, size_t N>
bool write(Writer& writer, const std::array<T, N>& value);

} // namespace yi::serialize::binary

#include "inl/write_inl.hpp" // IWYU pragma: keep
