#pragma once

// JSON 序列化库统一入口
// 自定义类型只需 include 此文件，并在 yi::serialize::json 命名空间中
// 为目标类型提供 read / write 两个重载即可。
//
// 亦可令类型定义以下成员：
//   void parse(Reader&)           — 手动读取各字段
//   void encode(Writer&) const    — 手动写入各字段
//   void __template_parse(Reader&)   — 代码生成的读取（优先于 parse）
//   void __template_encode(Writer&) const — 代码生成的写入（优先于 encode）
//
// 支持 JSONC（含注释的 JSON）：使用 parse_jsonc 替代 parse 即可。

// IWYU pragma: begin_exports
#include "detail/detail.h"  // IWYU pragma: keep
#include "detail/reader.h"  // IWYU pragma: keep
#include "detail/writer.h"  // IWYU pragma: keep
// IWYU pragma: end_exports

#include <string>
#include <tuple>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

namespace yi::serialize::json
{

// 删除 JSONC 注释（支持 // 单行注释和 /* */ 多行注释）
inline std::string remove_comments(const std::string& jsonc)
{
  std::string result;
  bool inString = false;
  for (size_t i = 0; i < jsonc.length(); ++i)
  {
    if (jsonc[i] == '\"')
      inString = !inString;
    if (!inString)
    {
      if (jsonc.substr(i, 2) == "//")
      {
        while (i < jsonc.length() && jsonc[i] != '\n')
          ++i;
        continue;
      }
      if (jsonc.substr(i, 2) == "/*")
      {
        while (i + 1 < jsonc.length() && jsonc.substr(i, 2) != "*/")
          ++i;
        ++i; // 跳过 "*/"
        continue;
      }
    }
    result += jsonc[i];
  }
  return result;
}

// 从 JSON 字符串反序列化对象
template<typename T>
bool parse(T& obj, const std::string& doc)
{
  rapidjson::Document jsonDoc;
  if (jsonDoc.Parse(doc.c_str(), doc.length()).HasParseError())
    return false;
  Reader reader{jsonDoc};
  return read(reader, obj);
}

// 从 JSONC（含注释）字符串反序列化对象
template<typename T>
bool parse_jsonc(T& obj, const std::string& doc)
{
  return parse(obj, remove_comments(doc));
}

// 将对象序列化为 JSON 字符串
template<typename T>
std::tuple<bool, std::string> encode(const T& obj)
{
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> w(buffer);
  Writer writer{w};
  if (!write(writer, obj))
    return {false, ""};
  return {true, buffer.GetString()};
}

} // namespace yi::serialize::json
