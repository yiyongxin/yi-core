#pragma once

#include <string>
#include <cstddef>
#include <vector>

namespace yi::serialize::binary
{

enum class string_coding
{
  gbk  = 0,
  utf8 = 1
};

// 变长数组模板
template <typename T, typename length_type>
struct vec : public std::vector<T> {};

// 变长字符串模板
template <typename length_type>
struct vstr : public std::string
{
  length_type _yi_core_serialize_binary_variable_length = 0;
};

// 定长字符串模板
template <size_t fixed_length>
struct fstr : public std::string {};

// ── 读写器模板 ─────────────────────────────────────────────────────────────
//
// 模板参数在编译期固定，消除运行时字节序/编码判断。
// 常规用法直接使用别名 Reader / Writer（小端 + UTF-8）。
// 非默认配置用 BasicReader<true> / BasicWriter<true, string_coding::gbk>。

// 读取器
// is_big_endian : 目标字节序（false = 小端）
// encoding      : 目标字符编码（程序内部始终 UTF-8；此参数描述外部格式）
template <bool is_big_endian, string_coding encoding>
struct BasicReader
{
  const char* buffer = nullptr;   // 只读缓冲区指针
  size_t      length = 0;         // 缓冲区总长度
  size_t      offset = 0;         // 当前读取偏移量
};

// 写入器
template <bool is_big_endian, string_coding encoding>
struct BasicWriter
{
  char*  buffer = nullptr;        // 可写缓冲区指针
  size_t length = 0;              // 缓冲区总长度
  size_t offset = 0;              // 当前写入偏移量
};

// ── 便捷别名 ───────────────────────────────────────────────────────────────

// 默认配置（小端 + UTF-8）：Reader r; / Writer w; 直接使用
using Reader = BasicReader<false, string_coding::utf8>;
using Writer = BasicWriter<false, string_coding::utf8>;

// 非默认配置：ReaderT<true> / WriterT<true, string_coding::gbk>
template <bool BE, string_coding ENC = string_coding::utf8>
using ReaderT = BasicReader<BE, ENC>;
template <bool BE, string_coding ENC = string_coding::utf8>
using WriterT = BasicWriter<BE, ENC>;

// 常用字符串类型别名
using str1b = vstr<uint8_t>;
using str2b = vstr<uint16_t>;
using str4b = vstr<uint32_t>;
using str8b = vstr<uint64_t>;
// 常用数组类型别名
template<typename T> using vec1b = vec<T,uint8_t>;
template<typename T> using vec2b = vec<T,uint16_t>;
template<typename T> using vec4b = vec<T,uint32_t>;
template<typename T> using vec8b = vec<T,uint64_t>;

} // namespace yi::serialize::binary
