#pragma once

#include <array>
#include <string>
#include <vector>
#include <cstdint>

#include "detail.h"

namespace yi::serialize::binary
{

// ── 基础类型 ──────────────────────────────────────────────────────────────

template<bool BE, string_coding ENC> bool read(BasicReader<BE,ENC>& r, uint8_t&  v);
template<bool BE, string_coding ENC> bool read(BasicReader<BE,ENC>& r, uint16_t& v);
template<bool BE, string_coding ENC> bool read(BasicReader<BE,ENC>& r, uint32_t& v);
template<bool BE, string_coding ENC> bool read(BasicReader<BE,ENC>& r, uint64_t& v);
template<bool BE, string_coding ENC> bool read(BasicReader<BE,ENC>& r, int8_t&   v);
template<bool BE, string_coding ENC> bool read(BasicReader<BE,ENC>& r, int16_t&  v);
template<bool BE, string_coding ENC> bool read(BasicReader<BE,ENC>& r, int32_t&  v);
template<bool BE, string_coding ENC> bool read(BasicReader<BE,ENC>& r, int64_t&  v);
template<bool BE, string_coding ENC> bool read(BasicReader<BE,ENC>& r, float&    v);
template<bool BE, string_coding ENC> bool read(BasicReader<BE,ENC>& r, double&   v);

// ── 字符串 ────────────────────────────────────────────────────────────────

template<bool BE, string_coding ENC>
bool read(BasicReader<BE,ENC>& r, std::string& v);

template<bool BE, string_coding ENC, typename length_type>
bool read(BasicReader<BE,ENC>& r, vstr<length_type>& v);

template<bool BE, string_coding ENC, size_t N>
bool read(BasicReader<BE,ENC>& r, fstr<N>& v);

// ── 容器 ──────────────────────────────────────────────────────────────────

template<bool BE, string_coding ENC, typename T>
bool read(BasicReader<BE,ENC>& r, std::vector<T>& v);

template<bool BE, string_coding ENC, typename T, typename length_type>
bool read(BasicReader<BE,ENC>& r, vec<T,length_type>& v);

template<bool BE, string_coding ENC, typename T, size_t N>
bool read(BasicReader<BE,ENC>& r, std::array<T,N>& v);

} // namespace yi::serialize::binary

#include "inl/read_inl.hpp" // IWYU pragma: keep
