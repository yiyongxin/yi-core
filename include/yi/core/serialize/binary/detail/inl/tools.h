#pragma once

#include <cstdint>

namespace yi::serialize::binary::detail
{

// 从小端缓冲区读取值（uint8_t 转换防止 char 符号扩展）
template<typename T>
inline T assemble_le(const char* p) {
  if constexpr (sizeof(T) == 1) {
    return static_cast<T>(p[0]);
  } else if constexpr (sizeof(T) == 2) {
    return static_cast<T>(
      (uint16_t(uint8_t(p[1])) << 8) | uint16_t(uint8_t(p[0])));
  } else if constexpr (sizeof(T) == 4) {
    return static_cast<T>(
      (uint32_t(uint8_t(p[3])) << 24) | (uint32_t(uint8_t(p[2])) << 16) |
      (uint32_t(uint8_t(p[1])) << 8)  |  uint32_t(uint8_t(p[0])));
  } else if constexpr (sizeof(T) == 8) {
    return static_cast<T>(
      (uint64_t(uint8_t(p[7])) << 56) | (uint64_t(uint8_t(p[6])) << 48) |
      (uint64_t(uint8_t(p[5])) << 40) | (uint64_t(uint8_t(p[4])) << 32) |
      (uint64_t(uint8_t(p[3])) << 24) | (uint64_t(uint8_t(p[2])) << 16) |
      (uint64_t(uint8_t(p[1])) << 8)  |  uint64_t(uint8_t(p[0])));
  }
}

}