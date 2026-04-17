#pragma once

#include <cstdint>
#include <memory>

namespace yi::common
{

struct Bytes
{
  int64_t length = 0;
  std::shared_ptr<char[]> data = nullptr;
};

struct Base64
{
  int64_t length = 0;
  std::shared_ptr<char[]> data = nullptr;
public:
  inline bool __template_string_parse(const std::string& str);
  inline std::string __template_string_encode() const;
};

// 工作台命名约定别名：yi::common::ByteBuffer → Bytes
using ByteBuffer = Bytes;

}