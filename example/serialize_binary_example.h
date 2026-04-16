#pragma once

#include "serialize/binary.h" // IWYU pragma: keep

namespace yi::example
{

struct TestStruct
{
  uint32_t    a;
  std::string b;
};

}

namespace yi::serialize::binary
{

inline size_t size(const yi::example::TestStruct& self)
{
  return size(self.a) +
         size(self.b);
}

inline bool read(Reader& reader, yi::example::TestStruct& self)
{
  if (!read(reader, self.a)) return false;
  if (!read(reader, self.b)) return false;
  return true;
}

inline bool write(Writer& writer, const yi::example::TestStruct& self)
{
  if (!write(writer, self.a)) return false;
  if (!write(writer, self.b)) return false;
  return true;
}

} // namespace yi::serialize::binary

inline void example_usage()
{
  char temp[1024] = {0};

  yi::example::TestStruct test{ 42, "hello" };

  // 写入
  yi::serialize::binary::Writer writer;
  writer.buffer = temp;
  writer.length = sizeof(temp);
  yi::serialize::binary::write(writer, test);

  // 读取
  yi::example::TestStruct result;
  yi::serialize::binary::Reader reader;
  reader.buffer = temp;
  reader.length = writer.offset;
  yi::serialize::binary::read(reader, result);
}
