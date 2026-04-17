#pragma once

#include "yi/core/serialize/json/json.h"  // IWYU pragma: keep

namespace yi::example
{

struct TestStruct
{
  uint32_t    a;
  std::string b;
};

}

namespace yi::serialize::json
{

inline bool read(Reader& reader, yi::example::TestStruct& self)
{
  read(reader, "a", self.a);
  read(reader, "b", self.b);
  return true;
}

inline bool write(Writer& writer, const yi::example::TestStruct& self)
{
  if (!write(writer, "a", self.a)) return false;
  if (!write(writer, "b", self.b)) return false;
  return true;
}

} // namespace yi::serialize::json

inline void example_json_usage()
{
  using namespace yi::serialize::json;

  yi::example::TestStruct test{42, "hello"};

  // 序列化
  auto [ok, json] = encode(test);

  // 反序列化
  yi::example::TestStruct result;
  parse(result, json);

  // JSONC（带注释）反序列化示例
  const std::string jsonc = R"({
    // 注释
    "a": 42,
    "b": "hello" /* 另一个注释 */
  })";
  yi::example::TestStruct result2;
  parse_jsonc(result2, jsonc);
}
