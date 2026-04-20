#pragma once

#include "yi/core/serialize/binary/binary.h"  // IWYU pragma: keep

#include <array>
#include <cstdint>
#include <string>
#include <vector>

using namespace std;
using namespace yi;
// ─── 示例结构体 ──────────────────────────────────────────────────────────────
//
// 每种结构体演示一类常见场景，搭配下方的 size/read/write 重载即可接入序列化。

namespace yi::example
{

// 二维坐标（纯整型字段）
struct Point
{
    int32_t x = 0;
    int32_t y = 0;
};

// 网络消息包（整型 + 标准字符串，4字节长度前缀）
struct Packet
{
    uint16_t    cmd  = 0;
    uint32_t    seq  = 0;
    std::string body;
};

// 玩家信息（使用 str2b：2字节长度前缀字符串）
struct PlayerInfo
{
    uint32_t                       id    = 0;
    yi::serialize::binary::str2b   name;   // 协议约定长度字段为 uint16_t
    float                          score = 0.0f;
};

// 协议头（使用 fstr<4>：4字节定长 magic 字段）
struct FixedHeader
{
    yi::serialize::binary::fstr<4>  magic;    // 固定 4 字节，如 "YICP"
    uint32_t                        version = 0;
};

// 关卡数据（使用 vec1b：1字节 count 前缀数组）
struct Level
{
    uint8_t                                     level_id  = 0;
    yi::serialize::binary::vec1b<Point>         obstacles;  // count 用 uint8_t 表示
};

// 空间变换（使用 std::array：定长数组，无 count 前缀）
struct Transform
{
    std::array<float, 3> position = {0.0f, 0.0f, 0.0f};
    std::array<float, 4> rotation = {0.0f, 0.0f, 0.0f, 1.0f};  // quaternion xyzw
};

} // namespace yi::example

// ─── 序列化重载 ──────────────────────────────────────────────────────────────
//
// 在 yi::serialize::binary 命名空间内为每个结构体提供三个重载：
//   size(const T&)          — 计算序列化字节数（用于预分配缓冲区）
//   write(Writer&, const T&)— 写入缓冲区，返回 false 表示空间不足
//   read(Reader&, T&)       — 从缓冲区读取，返回 false 表示数据不足

namespace yi::serialize::binary
{

// ── Point ──────────────────────────────────────────────────────────────────

inline size_t size(const yi::example::Point& self)
{
    return size(self.x) + size(self.y);
}
inline bool write(Writer& w, const yi::example::Point& self)
{
    return write(w, self.x) && write(w, self.y);
}
inline bool read(Reader& r, yi::example::Point& self)
{
    return read(r, self.x) && read(r, self.y);
}

// ── Packet ─────────────────────────────────────────────────────────────────

inline size_t size(const yi::example::Packet& self)
{
    return size(self.cmd) + size(self.seq) + size(self.body);
}
inline bool write(Writer& w, const yi::example::Packet& self)
{
    return write(w, self.cmd) && write(w, self.seq) && write(w, self.body);
}
inline bool read(Reader& r, yi::example::Packet& self)
{
    return read(r, self.cmd) && read(r, self.seq) && read(r, self.body);
}

// ── PlayerInfo ─────────────────────────────────────────────────────────────

inline size_t size(const yi::example::PlayerInfo& self)
{
    return size(self.id) + size(self.name) + size(self.score);
}
inline bool write(Writer& w, const yi::example::PlayerInfo& self)
{
    return write(w, self.id) && write(w, self.name) && write(w, self.score);
}
inline bool read(Reader& r, yi::example::PlayerInfo& self)
{
    return read(r, self.id) && read(r, self.name) && read(r, self.score);
}

// ── FixedHeader ────────────────────────────────────────────────────────────

inline size_t size(const yi::example::FixedHeader& self)
{
    return size(self.magic) + size(self.version);
}
inline bool write(Writer& w, const yi::example::FixedHeader& self)
{
    return write(w, self.magic) && write(w, self.version);
}
inline bool read(Reader& r, yi::example::FixedHeader& self)
{
    return read(r, self.magic) && read(r, self.version);
}

// ── Level ──────────────────────────────────────────────────────────────────

inline size_t size(const yi::example::Level& self)
{
    return size(self.level_id) + size(self.obstacles);
}
inline bool write(Writer& w, const yi::example::Level& self)
{
    return write(w, self.level_id) && write(w, self.obstacles);
}
inline bool read(Reader& r, yi::example::Level& self)
{
    return read(r, self.level_id) && read(r, self.obstacles);
}

// ── Transform ──────────────────────────────────────────────────────────────

inline size_t size(const yi::example::Transform& self)
{
    return size(self.position) + size(self.rotation);
}
inline bool write(Writer& w, const yi::example::Transform& self)
{
    return write(w, self.position) && write(w, self.rotation);
}
inline bool read(Reader& r, yi::example::Transform& self)
{
    return read(r, self.position) && read(r, self.rotation);
}

} // namespace yi::serialize::binary

// ─── 示例用法 ─────────────────────────────────────────────────────────────────

// 示例1：基础结构体往返（std::string 4字节长度前缀）
inline void example_basic_struct()
{
    using namespace yi::serialize::binary;

    yi::example::Packet pkt{0x0101u, 42u, "hello world"};

    char buf[256] = {};
    Writer w;
    w.buffer = buf;
    w.length = sizeof(buf);
    write(w, pkt);

    yi::example::Packet result;
    Reader r;
    r.buffer = buf;
    r.length = w.offset;
    read(r, result);
}

// 示例2：用 size() 预分配精确缓冲区，避免浪费
inline void example_exact_allocation()
{
    using namespace yi::serialize::binary;

    yi::example::Packet pkt{0x0202u, 99u, "exact"};
    std::vector<char> buf(size(pkt));

    Writer w;
    w.buffer = buf.data();
    w.length = buf.size();
    const bool ok = write(w, pkt);
    (void)ok;  // ok == true
}

// 示例3：结构体向量往返
inline void example_vector_of_structs()
{
    using namespace yi::serialize::binary;

    std::vector<yi::example::Point> pts{{1, 2}, {-3, 4}, {0, 0}};

    char buf[256] = {};
    Writer w;
    w.buffer = buf;
    w.length = sizeof(buf);
    write(w, pts);

    std::vector<yi::example::Point> result;
    Reader r;
    r.buffer = buf;
    r.length = w.offset;
    read(r, result);
}

// 示例4：vstr（自定义长度前缀）与 fstr（定长字符串）
inline void example_special_strings()
{
    using namespace yi::serialize::binary;

    // str2b：2字节长度前缀的字符串
    str2b name;
    name.assign("Alice");

    // fstr<4>：写入恰好 4 字节（内容不足时补零）
    fstr<4> magic;
    magic.assign("YICP");

    char buf[256] = {};
    Writer w;
    w.buffer = buf;
    w.length = sizeof(buf);
    write(w, name);   // 写入：2字节 + 5字节 = 7字节
    write(w, magic);  // 写入：固定 4字节

    str2b  name_out;
    fstr<4> magic_out;
    Reader r;
    r.buffer = buf;
    r.length = w.offset;
    read(r, name_out);
    read(r, magic_out);
}

// 示例5：vec<T, length_type>（自定义 count 前缀数组）
inline void example_custom_count_prefix()
{
    using namespace yi::serialize::binary;

    yi::example::Level lv;
    lv.level_id = 3;
    lv.obstacles.push_back({10, 20});
    lv.obstacles.push_back({-5, 15});

    char buf[256] = {};
    Writer w;
    w.buffer = buf;
    w.length = sizeof(buf);
    write(w, lv);  // count 字段只占 1 字节（uint8_t）

    yi::example::Level result;
    Reader r;
    r.buffer = buf;
    r.length = w.offset;
    read(r, result);
}

// 示例6：std::array（定长数组，无 count 前缀）
inline void example_fixed_array()
{
    using namespace yi::serialize::binary;

    yi::example::Transform xf;
    xf.position = {1.0f, 2.0f, 3.0f};
    xf.rotation = {0.0f, 0.0f, 0.707f, 0.707f};

    std::vector<char> buf(size(xf));
    Writer w;
    w.buffer = buf.data();
    w.length = buf.size();
    write(w, xf);

    yi::example::Transform result;
    Reader r;
    r.buffer = buf.data();
    r.length = w.offset;
    read(r, result);
}

// 示例7：大端字节序（WriterT<true> / ReaderT<true>）
inline void example_big_endian()
{
    using namespace yi::serialize::binary;

    char buf[32] = {};
    WriterT<true> w;  // 大端写入器
    w.buffer = buf;
    w.length = sizeof(buf);
    write(w, uint32_t{0x01020304u});
    // buf = {0x01, 0x02, 0x03, 0x04}

    uint32_t val = 0;
    ReaderT<true> r;  // 大端读取器
    r.buffer = buf;
    r.length = w.offset;
    read(r, val);
    // val == 0x01020304u
    (void)val;
}

// 示例8：错误处理——缓冲区不足时 write 返回 false，offset 不前进
inline void example_overflow_handling()
{
    using namespace yi::serialize::binary;

    char buf[3] = {};  // 故意给不够大的缓冲区
    Writer w;
    w.buffer = buf;
    w.length = sizeof(buf);

    const bool ok = write(w, uint32_t{42u});  // 需要 4 字节，只有 3 字节
    (void)ok;  // ok == false，w.offset == 0（未前进）
}
