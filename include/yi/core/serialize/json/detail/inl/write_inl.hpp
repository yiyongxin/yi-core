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

#include "../detail.h"

namespace yi::serialize::json
{

//------------------------------基础类型支持---------------------------------

inline bool write(Writer& writer, bool         value) { return writer.writer.Bool(value);         }
inline bool write(Writer& writer, double        value) { return writer.writer.Double(value);       }
inline bool write(Writer& writer, float         value) { return writer.writer.Double(value);       }
inline bool write(Writer& writer, uint8_t       value) { return writer.writer.Uint(value);         }
inline bool write(Writer& writer, uint16_t      value) { return writer.writer.Uint(value);         }
inline bool write(Writer& writer, uint32_t      value) { return writer.writer.Uint(value);         }
inline bool write(Writer& writer, uint64_t      value) { return writer.writer.Uint64(value);       }
inline bool write(Writer& writer, int8_t        value) { return writer.writer.Int(value);          }
inline bool write(Writer& writer, int16_t       value) { return writer.writer.Int(value);          }
inline bool write(Writer& writer, int32_t       value) { return writer.writer.Int(value);          }
inline bool write(Writer& writer, int64_t       value) { return writer.writer.Int64(value);        }

inline bool write(Writer& writer, const std::string& value)
{
  return writer.writer.String(value.c_str());
}

inline bool write(Writer& writer, const char* value)
{
  return writer.writer.String(value);
}

inline bool write(Writer& writer, const std::chrono::seconds& value)
{
  return writer.writer.Int64(value.count());
}

//------------------------------容器类型支持---------------------------------

template<typename T, std::size_t N>
inline bool write(Writer& writer, const std::array<T, N>& value)
{
  if (!writer.writer.StartArray()) return false;
  for (const auto& item : value)
    if (!write(writer, item)) return false;
  if (!writer.writer.EndArray()) return false;
  return true;
}

template<typename T>
inline bool write(Writer& writer, const std::list<T>& value)
{
  if (!writer.writer.StartArray()) return false;
  for (const auto& item : value)
    if (!write(writer, item)) return false;
  if (!writer.writer.EndArray()) return false;
  return true;
}

template<typename T>
inline bool write(Writer& writer, const std::vector<T>& value)
{
  if (!writer.writer.StartArray()) return false;
  for (const auto& item : value)
    if (!write(writer, item)) return false;
  if (!writer.writer.EndArray()) return false;
  return true;
}

template<typename T>
inline bool write(Writer& writer, const std::set<T>& value)
{
  if (!writer.writer.StartArray()) return false;
  for (const auto& item : value)
    if (!write(writer, item)) return false;
  if (!writer.writer.EndArray()) return false;
  return true;
}

template<typename T>
inline bool write(Writer& writer, const std::optional<T>& value)
{
  if (value.has_value())
    return write(writer, value.value());
  return true;
}

template<typename T>
inline bool write(Writer& writer, const std::map<std::string, T>& value)
{
  if (!writer.writer.StartObject()) return false;
  for (const auto& [k, v] : value)
  {
    if (!writer.writer.Key(k.c_str())) return false;
    if (!write(writer, v)) return false;
  }
  if (!writer.writer.EndObject()) return false;
  return true;
}

template<typename T>
inline bool write(Writer& writer, const std::unordered_map<std::string, T>& value)
{
  if (!writer.writer.StartObject()) return false;
  for (const auto& [k, v] : value)
  {
    if (!writer.writer.Key(k.c_str())) return false;
    if (!write(writer, v)) return false;
  }
  if (!writer.writer.EndObject()) return false;
  return true;
}

template<typename T>
inline bool write(Writer& writer, const std::map<int64_t, T>& value)
{
  if (!writer.writer.StartObject()) return false;
  for (const auto& [k, v] : value)
  {
    const std::string key = std::to_string(k);
    if (!writer.writer.Key(key.c_str())) return false;
    if (!write(writer, v)) return false;
  }
  if (!writer.writer.EndObject()) return false;
  return true;
}

template<typename T>
inline bool write(Writer& writer, const std::unordered_map<int64_t, T>& value)
{
  if (!writer.writer.StartObject()) return false;
  for (const auto& [k, v] : value)
  {
    const std::string key = std::to_string(k);
    if (!writer.writer.Key(key.c_str())) return false;
    if (!write(writer, v)) return false;
  }
  if (!writer.writer.EndObject()) return false;
  return true;
}

//------------------------------对象类型支持---------------------------------

template<NotOptional T>
inline bool write(Writer& writer, const T& value)
{
  if constexpr (has_template_string_encode<T> || has_template_number_encode<T>)
  {
    if constexpr (has_template_string_encode<T>)
    {
      const std::string temp = value.__template_string_encode();
      return writer.writer.String(temp.c_str());
    }
    if constexpr (has_template_number_encode<T>)
    {
      return writer.writer.Int64(value.__template_number_encode());
    }
  }
  else
  {
    if (!writer.writer.StartObject()) return false;
    if constexpr (has_template_encode<T>)
      value.__template_encode(writer);
    else if constexpr (has_encode<T>)
      value.encode(writer);
    else
    {
      if (!wrapper<T>::write(value, writer)) return false;
    }
    if (!writer.writer.EndObject()) return false;
  }
  return true;
}

//------------------------------键值写入---------------------------------

template<typename T>
inline bool write(Writer& writer, const char* name, const T& value)
{
  if (!writer.writer.Key(name)) return false;
  return write(writer, value);
}

template<typename T>
inline bool write(Writer& writer, const char* name, T& value)
{
  if (!writer.writer.Key(name)) return false;
  return write(writer, value);
}

} // namespace yi::serialize::json
