#pragma once

#include <string>
#include <cstddef>
#include <vector>

namespace yi::serialize::binary
{

enum class string_coding
{
  gbk = 0,
  utf8 = 1
};

//变长数组模板
template <typename T, typename length_type>
struct vec : public std::vector<T>{};

//变长字符串模板
template <typename length_type>
struct vstr : public std::string
{
  length_type _yi_serialize_binary_variable_length = 0; // 字符串长度(一般不会用这个成员变量，故意把名字写的很难看)
};

//定长字符串模板
template <size_t fixed_length>
struct fstr : public std::string{};

struct Reader
{
  const char* buffer = nullptr;                 // 只读缓冲区指针
  size_t length = 0;                            // 缓冲区总长度
  size_t offset = 0;                            // 当前读取偏移量
  string_coding encoding = string_coding::gbk;  // 字符编码 (程序内部使用utf8编码，这里的编码是指目标协议字符串编码)
  bool is_big_endian = false;                   // 是否大端字节序
};

struct Writer
{
  char* buffer = nullptr;                       // 可写缓冲区指针
  size_t length = 0;                            // 缓冲区总长度
  size_t offset = 0;                            // 当前写入偏移量
  string_coding encoding = string_coding::gbk;  // 字符编码 (程序内部使用utf8编码，这里的编码是指目标协议字符串编码)
  bool is_big_endian = false;                   // 是否大端字节序
};

}