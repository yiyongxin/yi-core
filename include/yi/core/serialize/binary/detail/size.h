#pragma once

#include <array>
#include <string>
#include <vector>
#include <cstdint>
#include <cstddef>

#include "detail.h"

namespace yi::serialize::binary
{
//------------------------------基础类型支持---------------------------------

constexpr size_t size(const uint8_t&);
constexpr size_t size(const uint16_t&);
constexpr size_t size(const uint32_t&);
constexpr size_t size(const uint64_t&);
constexpr size_t size(const int8_t&);
constexpr size_t size(const int16_t&);
constexpr size_t size(const int32_t&);
constexpr size_t size(const int64_t&);
constexpr size_t size(const float&);
constexpr size_t size(const double&);
//-------------------------------字符串支持-------------------------------

// 默认4字节长度前缀的变长字符串
size_t size(const std::string& value);
// 自定义长度前缀的变长字符串
template <typename length_type>
size_t size(const vstr<length_type>& value);
// 定长字符串（编译期常量）
template <size_t N>
constexpr size_t size(const fstr<N>& value);
//-------------------------------容器类型支持-------------------------------

//读取变长数组 默认4字节可变长数组
template<typename T>
size_t size(const std::vector<T>& value);
//读取变长数组
template <typename T, typename length_type>
size_t size(const vec<T,length_type>& value);
//读取定长数组
template<typename T, size_t N>
size_t size(const std::array<T,N>& value);

} // namespace yi::serialize::binary

#include "inl/size_inl.hpp" // IWYU pragma: keep
