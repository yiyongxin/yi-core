#pragma once

#include <array>
#include <chrono>
#include <list>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

#include "detail.h"

namespace yi::serialize::json
{

//------------------------------基础类型支持---------------------------------

bool write(Writer& writer, bool          value);  // 写入 bool 值
bool write(Writer& writer, double        value);  // 写入 double 值
bool write(Writer& writer, float         value);  // 写入 float 值
bool write(Writer& writer, uint8_t       value);  // 写入 uint8 值
bool write(Writer& writer, uint16_t      value);  // 写入 uint16 值
bool write(Writer& writer, uint32_t      value);  // 写入 uint32 值
bool write(Writer& writer, uint64_t      value);  // 写入 uint64 值
bool write(Writer& writer, int8_t        value);  // 写入 int8 值
bool write(Writer& writer, int16_t       value);  // 写入 int16 值
bool write(Writer& writer, int32_t       value);  // 写入 int32 值
bool write(Writer& writer, int64_t       value);  // 写入 int64 值
bool write(Writer& writer, const std::string&          value);  // 写入字符串
bool write(Writer& writer, const char*                 value);  // 写入 C 字符串
bool write(Writer& writer, const std::chrono::seconds& value);  // 写入秒数

//------------------------------容器类型支持---------------------------------

// 写入定长数组
template<typename T, std::size_t N>
bool write(Writer& writer, const std::array<T, N>& value);
// 写入链表
template<typename T>
bool write(Writer& writer, const std::list<T>& value);
// 写入变长数组
template<typename T>
bool write(Writer& writer, const std::vector<T>& value);
// 写入集合
template<typename T>
bool write(Writer& writer, const std::set<T>& value);
// 写入 std::optional<T>
template<typename T>
bool write(Writer& writer, const std::optional<T>& value);
// 写入映射表 (string key)
template<typename T>
bool write(Writer& writer, const std::map<std::string, T>& value);
// 写入映射表 (string key, unordered)
template<typename T>
bool write(Writer& writer, const std::unordered_map<std::string, T>& value);
// 写入映射表 (int64 key)
template<typename T>
bool write(Writer& writer, const std::map<int64_t, T>& value);
// 写入映射表 (int64 key, unordered)
template<typename T>
bool write(Writer& writer, const std::unordered_map<int64_t, T>& value);

//------------------------------对象类型支持---------------------------------

// 写入泛型对象（通过 __template_encode / encode / wrapper 分发）
template<NotOptional T>
bool write(Writer& writer, const T& value);

//------------------------------键值写入---------------------------------

// 写入 JSON 对象键值对
template<typename T>
bool write(Writer& writer, const char* name, const T& value);
template<typename T>
bool write(Writer& writer, const char* name, T& value);

} // namespace yi::serialize::json

#include "inl/write_inl.hpp"  // IWYU pragma: keep
