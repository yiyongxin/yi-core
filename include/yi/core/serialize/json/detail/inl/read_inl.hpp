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

#include "../detail.h"

namespace yi::serialize::json
{

//------------------------------基础类型支持---------------------------------

inline bool read(Reader& reader, bool& value)
{
  if (!reader.value.IsBool()) return false;
  value = reader.value.GetBool();
  return true;
}

inline bool read(Reader& reader, double& value)
{
  if (!reader.value.IsNumber()) return false;
  value = reader.value.GetDouble();
  return true;
}

inline bool read(Reader& reader, float& value)
{
  if (!reader.value.IsNumber()) return false;
  value = reader.value.GetFloat();
  return true;
}

inline bool read(Reader& reader, uint8_t& value)
{
  if (!reader.value.IsUint()) return false;
  value = static_cast<uint8_t>(reader.value.GetUint());
  return true;
}

inline bool read(Reader& reader, uint16_t& value)
{
  if (!reader.value.IsUint()) return false;
  value = static_cast<uint16_t>(reader.value.GetUint());
  return true;
}

inline bool read(Reader& reader, uint32_t& value)
{
  if (!reader.value.IsUint()) return false;
  value = reader.value.GetUint();
  return true;
}

inline bool read(Reader& reader, uint64_t& value)
{
  if (!reader.value.IsUint64()) return false;
  value = reader.value.GetUint64();
  return true;
}

inline bool read(Reader& reader, int8_t& value)
{
  if (!reader.value.IsInt()) return false;
  value = static_cast<int8_t>(reader.value.GetInt());
  return true;
}

inline bool read(Reader& reader, int16_t& value)
{
  if (!reader.value.IsInt()) return false;
  value = static_cast<int16_t>(reader.value.GetInt());
  return true;
}

inline bool read(Reader& reader, int32_t& value)
{
  if (!reader.value.IsInt()) return false;
  value = reader.value.GetInt();
  return true;
}

inline bool read(Reader& reader, int64_t& value)
{
  if (!reader.value.IsInt64()) return false;
  value = reader.value.GetInt64();
  return true;
}

inline bool read(Reader& reader, std::string& value)
{
  if (!reader.value.IsString()) return false;
  const char* temp = reader.value.GetString();
  value = (temp != nullptr) ? temp : "";
  return true;
}

inline bool read(Reader& reader, std::chrono::seconds& value)
{
  if (!reader.value.IsNumber()) return false;
  value = std::chrono::seconds(reader.value.GetInt64());
  return true;
}

//------------------------------容器类型支持---------------------------------

template<typename T, std::size_t N>
inline bool read(Reader& reader, std::array<T, N>& value)
{
  if (!reader.value.IsArray()) return false;
  auto&& array = reader.value.GetArray();
  std::size_t count = 0;
  for (auto it = array.Begin(); it != array.End() && count < N; ++it, ++count)
  {
    Reader sub{*it};
    if (!read(sub, value[count]))
      return false;
  }
  return true;
}

template<typename T>
inline bool read(Reader& reader, std::list<T>& value)
{
  if (!reader.value.IsArray()) return false;
  auto&& array = reader.value.GetArray();
  for (auto it = array.Begin(); it != array.End(); ++it)
  {
    Reader sub{*it};
    if (!read(sub, value.emplace_back()))
      return false;
  }
  return true;
}

template<typename T>
inline bool read(Reader& reader, std::vector<T>& value)
{
  if (!reader.value.IsArray()) return false;
  auto&& array = reader.value.GetArray();
  value.reserve(array.Size());
  for (auto it = array.Begin(); it != array.End(); ++it)
  {
    Reader sub{*it};
    if (!read(sub, value.emplace_back()))
      return false;
  }
  return true;
}

template<typename T>
inline bool read(Reader& reader, std::set<T>& value)
{
  if (!reader.value.IsArray()) return false;
  auto&& array = reader.value.GetArray();
  for (auto it = array.Begin(); it != array.End(); ++it)
  {
    T item{};
    Reader sub{*it};
    if (!read(sub, item))
      return false;
    value.insert(std::move(item));
  }
  return true;
}

template<typename T>
inline bool read(Reader& reader, std::optional<T>& value)
{
  value.emplace();
  if (!read(reader, value.value()))
  {
    value.reset();
    return false;
  }
  return true;
}

template<typename T>
inline bool read(Reader& reader, std::map<std::string, T>& value)
{
  if (!reader.value.IsObject()) return false;
  auto&& obj = reader.value.GetObject();
  for (auto it = obj.MemberBegin(); it != obj.MemberEnd(); ++it)
  {
    if (!it->name.IsString()) continue;
    T item{};
    Reader sub{it->value};
    if (!read(sub, item)) continue;
    value[it->name.GetString()] = std::move(item);
  }
  return true;
}

template<typename T>
inline bool read(Reader& reader, std::map<int64_t, T>& value)
{
  if (!reader.value.IsObject()) return false;
  auto&& obj = reader.value.GetObject();
  for (auto it = obj.MemberBegin(); it != obj.MemberEnd(); ++it)
  {
    if (!it->name.IsString()) continue;
    T item{};
    Reader sub{it->value};
    if (!read(sub, item)) continue;
    int64_t key = std::stoll(it->name.GetString());
    value[key] = std::move(item);
  }
  return true;
}

//------------------------------对象类型支持---------------------------------

template<NotOptional T>
inline bool read(Reader& reader, T& value)
{
  if constexpr (has_template_string_parse<T> || has_template_number_parse<T>)
  {
    if constexpr (has_template_string_parse<T>)
    {
      if (reader.value.IsString())
        return value.__template_string_parse(reader.value.GetString());
    }
    if constexpr (has_template_number_parse<T>)
    {
      if (reader.value.IsInt64())
        return value.__template_number_parse(reader.value.GetInt64());
    }
    return false;
  }
  else
  {
    if (!reader.value.IsObject()) return false;
    if constexpr (has_template_parse<T>)
      value.__template_parse(reader);
    else if constexpr (has_parse<T>)
      value.parse(reader);
    else
      return wrapper<T>::read(value, reader);
    return true;
  }
}

//------------------------------键值读取---------------------------------

template<typename T>
inline bool read(Reader& reader, const char* name, T& value)
{
  if (!reader.value.HasMember(name)) return false;
  Reader sub{reader.value[name]};
  return read(sub, value);
}

} // namespace yi::serialize::json
