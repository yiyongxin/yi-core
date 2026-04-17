#pragma once

#include <array>
#include <string>
#include <vector>
#include <cstdint>

#include "detail.h"

namespace yi::serialize::binary
{

//------------------------------基础类型支持---------------------------------

template<bool BE, string_coding ENC> bool write(BasicWriter<BE,ENC>& w, const uint8_t&  v);
template<bool BE, string_coding ENC> bool write(BasicWriter<BE,ENC>& w, const uint16_t& v);
template<bool BE, string_coding ENC> bool write(BasicWriter<BE,ENC>& w, const uint32_t& v);
template<bool BE, string_coding ENC> bool write(BasicWriter<BE,ENC>& w, const uint64_t& v);
template<bool BE, string_coding ENC> bool write(BasicWriter<BE,ENC>& w, const int8_t&   v);
template<bool BE, string_coding ENC> bool write(BasicWriter<BE,ENC>& w, const int16_t&  v);
template<bool BE, string_coding ENC> bool write(BasicWriter<BE,ENC>& w, const int32_t&  v);
template<bool BE, string_coding ENC> bool write(BasicWriter<BE,ENC>& w, const int64_t&  v);
template<bool BE, string_coding ENC> bool write(BasicWriter<BE,ENC>& w, const float&    v);
template<bool BE, string_coding ENC> bool write(BasicWriter<BE,ENC>& w, const double&   v);

//-------------------------------字符串支持-------------------------------

template<bool BE, string_coding ENC>
bool write(BasicWriter<BE,ENC>& w, const std::string& v);

template<bool BE, string_coding ENC, typename length_type>
bool write(BasicWriter<BE,ENC>& w, const vstr<length_type>& v);

template<bool BE, string_coding ENC, size_t N>
bool write(BasicWriter<BE,ENC>& w, const fstr<N>& v);

//-------------------------------容器类型支持-------------------------------

template<bool BE, string_coding ENC, typename T>
bool write(BasicWriter<BE,ENC>& w, const std::vector<T>& v);

template<bool BE, string_coding ENC, typename T, typename length_type>
bool write(BasicWriter<BE,ENC>& w, const vec<T,length_type>& v);

template<bool BE, string_coding ENC, typename T, size_t N>
bool write(BasicWriter<BE,ENC>& w, const std::array<T,N>& v);

} // namespace yi::serialize::binary

#include "inl/write_inl.hpp" // IWYU pragma: keep
