#pragma once

#include <array>
#include <chrono>
#include <list>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>
#include <cstdint>

#include "detail.h"

namespace yi::serialize::json
{

//------------------------------基础类型支持---------------------------------

bool read(Reader& reader, bool&     value);  // 读取 bool 值
bool read(Reader& reader, double&   value);  // 读取 double 值
bool read(Reader& reader, float&    value);  // 读取 float 值
bool read(Reader& reader, uint8_t&  value);  // 读取 uint8 值
bool read(Reader& reader, uint16_t& value);  // 读取 uint16 值
bool read(Reader& reader, uint32_t& value);  // 读取 uint32 值
bool read(Reader& reader, uint64_t& value);  // 读取 uint64 值
bool read(Reader& reader, int8_t&   value);  // 读取 int8 值
bool read(Reader& reader, int16_t&  value);  // 读取 int16 值
bool read(Reader& reader, int32_t&  value);  // 读取 int32 值
bool read(Reader& reader, int64_t&  value);  // 读取 int64 值
bool read(Reader& reader, std::string&         value);  // 读取字符串
bool read(Reader& reader, std::chrono::seconds& value); // 读取秒数

//------------------------------容器类型支持---------------------------------

// 读取定长数组
template<typename T, std::size_t N>
bool read(Reader& reader, std::array<T, N>& value);
// 读取链表
template<typename T>
bool read(Reader& reader, std::list<T>& value);
// 读取变长数组
template<typename T>
bool read(Reader& reader, std::vector<T>& value);
// 读取集合
template<typename T>
bool read(Reader& reader, std::set<T>& value);
// 读取 std::optional<T>
template<typename T>
bool read(Reader& reader, std::optional<T>& value);
// 读取映射表 (string key)
template<typename T>
bool read(Reader& reader, std::map<std::string, T>& value);
// 读取映射表 (int64 key)
template<typename T>
bool read(Reader& reader, std::map<int64_t, T>& value);

//------------------------------对象类型支持---------------------------------

// 读取泛型对象（通过 __template_parse / parse / wrapper 分发）
template<NotOptional T>
bool read(Reader& reader, T& value);

//------------------------------键值读取---------------------------------

// 读取 JSON 对象中的指定键
template<typename T>
bool read(Reader& reader, const char* name, T& value);

} // namespace yi::serialize::json

#include "inl/read_inl.hpp"  // IWYU pragma: keep
