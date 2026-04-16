#pragma once

#include <string>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

namespace yi::serialize::json
{

struct Reader
{
  const rapidjson::Value& value;  // 当前正在读取的 JSON 节点
};

struct Writer
{
  rapidjson::Writer<rapidjson::StringBuffer>& writer;  // JSON 序列化器
};

// wrapper 模板：为无法直接定义 read/write 重载的类型提供回退机制
template<typename T>
struct wrapper
{
  static bool read(T& self, Reader& reader);
  static bool write(const T& self, Writer& writer);
};

//------------------------------辅助概念---------------------------------

// 读取侧：检测自定义类型是否定义了 parse / __template_parse 成员
template<typename T>
concept has_parse = requires(T t, Reader& r) { { t.parse(r) }; };

template<typename T>
concept has_template_parse = requires(T t, Reader& r) { { t.__template_parse(r) }; };

template<typename T>
concept has_template_string_parse = requires(T t, const std::string& s) { { t.__template_string_parse(s) }; };

template<typename T>
concept has_template_number_parse = requires(T t, const int64_t& n) { { t.__template_number_parse(n) }; };

template<typename T>
concept has_wrapper_read = requires(T value, Reader& reader) { wrapper<T>::read(value, reader); };

// 写入侧：检测自定义类型是否定义了 encode / __template_encode 成员
template<typename T>
concept has_encode = requires(T t, Writer& w) { { t.encode(w) }; };

template<typename T>
concept has_template_encode = requires(T t, Writer& w) { { t.__template_encode(w) }; };

template<typename T>
concept has_template_string_encode = requires(T t) { { t.__template_string_encode() }; };

template<typename T>
concept has_template_number_encode = requires(T t) { { t.__template_number_encode() }; };

template<typename T>
concept has_wrapper_write = requires(T value, Writer& writer) { wrapper<T>::write(value, writer); };

// 排除 std::optional（optional 不参与泛型对象分发）
template<class T>
concept NotOptional = !requires {
  typename T::value_type;
  T{}.has_value();
};

} // namespace yi::serialize::json
