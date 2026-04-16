#pragma once

#include <array>
#include <string>
#include <vector>
#include <cstdint>

#include "detail.h"

namespace yi::serialize::binary
{

//------------------------------基础类型支持---------------------------------

bool read(Reader& reader, uint8_t& value);  //读取uint8值
bool read(Reader& reader, uint16_t& value); //读取uint16_t值
bool read(Reader& reader, uint32_t& value); //读取uint32_t值
bool read(Reader& reader, uint64_t& value); //读取uint64_t值
bool read(Reader& reader, int8_t& value);   //读取int8_t值
bool read(Reader& reader, int16_t& value);  //读取int16_t值
bool read(Reader& reader, int32_t& value);  //读取int32_t值
bool read(Reader& reader, int64_t& value);  //读取int64_t值
bool read(Reader& reader, float& value);    //读取float值
bool read(Reader& reader, double& value);   //读取double值
//-------------------------------字符串支持-------------------------------

//读取字符串 默认4字节可变长字符串
bool read(Reader& reader, std::string& value);
//读取变长字符串
template <typename length_type>
bool read(Reader& reader, vstr<length_type>& value);
//读取定长字符串
template <size_t fixed_length>
bool read(Reader& reader, fstr<fixed_length>& value);
//-------------------------------容器类型支持-------------------------------

//读取变长数组 默认4字节可变长数组
template<typename T>
bool read(Reader& reader, std::vector<T>& value);
//读取变长数组
template <typename T, typename length_type>
bool read(Reader& reader, vec<T,length_type>& value);
//读取定长数组
template<typename T, size_t N>
bool read(Reader& reader, std::array<T,N>& value);

}

#include "inl/read_inl.hpp" // IWYU pragma: keep