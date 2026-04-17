// 二进制序列化单元测试
//
// BinarySize
//   ScalarTypes          — uint8/16/32/64、int8/16/32/64、float、double 的 size() 返回值
//   StdString            — 空字符串和非空字符串的 size()（含 4 字节长度前缀）
//   StdVector            — 空向量、uint32_t 向量、string 向量的 size()
//
// BinaryRoundTrip
//   UInt8/16/32/64       — 无符号整型写入再读回，值与字节数正确
//   Int8/16/32/64        — 有符号整型写入再读回，含边界值
//   Float_LE             — float 小端往返，含最大/最小值
//   Double_LE            — double 小端往返，含最大/最小值
//   FloatWritesByteCount — float 写入恰好消耗 4 字节
//   DoubleWritesByteCount— double 写入恰好消耗 8 字节
//   FloatZeroRoundTrip   — 0.0f 特殊位模式往返
//   DoubleZeroRoundTrip  — 0.0 特殊位模式往返
//   StdStringEmpty       — 空字符串往返，写入 4 字节
//   StdStringNormal      — 普通字符串往返
//   StdStringWithSpaces  — 含空格字符串往返
//   StdStringLong        — 512 字节长字符串往返
//   VectorEmpty          — 空向量往返，写入 4 字节
//   VectorUInt32         — uint32_t 向量往返
//   VectorInt32Negative  — 含负数的 int32_t 向量往返
//   VectorStrings        — string 向量往返
//   NestedVector         — vector<vector<int32_t>> 嵌套往返
//   CustomStruct_Point   — 自定义结构体 Point（含手动重载）往返
//   CustomStruct_Packet  — 自定义结构体 Packet（含字符串字段）往返
//   VectorOfStructs      — Point 向量往返
//
// BinaryOverflow
//   WriteFailsOnTooSmallBuffer   — 缓冲区不足时 write 返回 false，offset 不前进
//   WriteExactFit                — 恰好填满缓冲区时 write 成功
//   WriteStringTooLong           — 字符串数据超出缓冲区时 write 失败
//   WriteStringLengthPrefixFits  — 连长度前缀都放不下时 write 失败
//   ReadFailsOnTruncatedBuffer   — 缓冲区截断时 read 返回 false
//   ReadStringDataTruncated      — 字符串数据不完整时 read 失败
//   MultipleWritesExhaustBuffer  — 连续写入耗尽缓冲区后再写入失败
//
// BinaryEndian
//   UInt16LittleEndian           — uint16_t 小端字节序验证
//   UInt32LittleEndian           — uint32_t 小端字节序验证
//   UInt64LittleEndian           — uint64_t 小端字节序验证
//   StringLengthPrefixLittleEndian — 字符串长度前缀小端写入验证
//
// BinaryOffset
//   OffsetAdvancesCorrectly      — Writer.offset 随每次写入正确递增
//   ReaderOffsetAdvancesCorrectly— Reader.offset 随每次读取正确递增

#include <gtest/gtest.h>
#include "yi/core/serialize/binary/binary.h"

#include <cstdint>
#include <limits>
#include <string>
#include <vector>

using namespace yi::serialize::binary;

// ── 辅助工具 ────────────────────────────────────────────────────────────────

namespace {

// 将 T 写入栈上缓冲区，再读回，返回结果值及各阶段标志
template <typename T>
struct RoundTrip {
    bool   write_ok     = false;
    bool   read_ok      = false;
    size_t bytes_written = 0;
    T      value        = {};
};

template <typename T>
RoundTrip<T> roundtrip(const T& in, size_t buf_capacity = 1024)
{
    std::vector<char> buf(buf_capacity, 0);

    Writer w;
    w.buffer = buf.data();
    w.length = buf.size();

    RoundTrip<T> res;
    res.write_ok     = write(w, in);
    res.bytes_written = w.offset;

    Reader r;
    r.buffer = buf.data();
    r.length = w.offset;

    T out{};
    res.read_ok = read(r, out);
    res.value   = out;
    return res;
}

// 取缓冲区第 idx 字节（无符号）
inline uint8_t byte_at(const char* buf, size_t idx)
{
    return static_cast<uint8_t>(buf[idx]);
}

} // namespace

// ═══════════════════════════════════════════════════════════════════════════
// size() 正确性
// ═══════════════════════════════════════════════════════════════════════════

TEST(BinarySize, ScalarTypes)
{
    EXPECT_EQ(size(uint8_t{}),  1u);
    EXPECT_EQ(size(uint16_t{}), 2u);
    EXPECT_EQ(size(uint32_t{}), 4u);
    EXPECT_EQ(size(uint64_t{}), 8u);
    EXPECT_EQ(size(int8_t{}),   1u);
    EXPECT_EQ(size(int16_t{}),  2u);
    EXPECT_EQ(size(int32_t{}),  4u);
    EXPECT_EQ(size(int64_t{}),  8u);
    EXPECT_EQ(size(float{}),    4u);
    EXPECT_EQ(size(double{}),   8u);
}

TEST(BinarySize, StdString)
{
    // 空字符串：只有 4 字节长度前缀
    EXPECT_EQ(size(std::string{}), sizeof(uint32_t));
    // 5 字符：4 + 5
    EXPECT_EQ(size(std::string{"hello"}), sizeof(uint32_t) + 5u);
    // 100 字符
    std::string s(100, 'x');
    EXPECT_EQ(size(s), sizeof(uint32_t) + 100u);
}

TEST(BinarySize, StdVector)
{
    // 空向量：只有 4 字节元素数前缀
    EXPECT_EQ(size(std::vector<uint32_t>{}), sizeof(uint32_t));
    // 3 个 uint32_t：4 + 3*4
    std::vector<uint32_t> v3{1u, 2u, 3u};
    EXPECT_EQ(size(v3), sizeof(uint32_t) + 3 * sizeof(uint32_t));
    // 包含字符串的向量
    std::vector<std::string> vs{"ab", "cde"};
    // 4 (count) + (4+2) + (4+3)
    EXPECT_EQ(size(vs), 4u + (4u + 2u) + (4u + 3u));
}

// ═══════════════════════════════════════════════════════════════════════════
// 无符号整型 – 写入/读取往返
// ═══════════════════════════════════════════════════════════════════════════

TEST(BinaryRoundTrip, UInt8)
{
    for (uint8_t v : {uint8_t(0), uint8_t(1), uint8_t(127), uint8_t(255)}) {
        auto res = roundtrip(v);
        EXPECT_TRUE(res.write_ok) << "write failed for v=" << unsigned(v);
        EXPECT_TRUE(res.read_ok)  << "read failed for v="  << unsigned(v);
        EXPECT_EQ(res.value, v);
        EXPECT_EQ(res.bytes_written, 1u);
    }
}

TEST(BinaryRoundTrip, UInt16)
{
    for (uint16_t v : {uint16_t(0), uint16_t(1), uint16_t(0x1234), uint16_t(0xFFFF)}) {
        auto res = roundtrip(v);
        EXPECT_TRUE(res.write_ok);
        EXPECT_TRUE(res.read_ok);
        EXPECT_EQ(res.value, v);
        EXPECT_EQ(res.bytes_written, 2u);
    }
}

TEST(BinaryRoundTrip, UInt32)
{
    for (uint32_t v : {0u, 1u, 0x12345678u, std::numeric_limits<uint32_t>::max()}) {
        auto res = roundtrip(v);
        EXPECT_TRUE(res.write_ok);
        EXPECT_TRUE(res.read_ok);
        EXPECT_EQ(res.value, v);
        EXPECT_EQ(res.bytes_written, 4u);
    }
}

TEST(BinaryRoundTrip, UInt64)
{
    for (uint64_t v : {uint64_t(0), uint64_t(1), uint64_t(0x0123456789ABCDEFull),
                       std::numeric_limits<uint64_t>::max()}) {
        auto res = roundtrip(v);
        EXPECT_TRUE(res.write_ok);
        EXPECT_TRUE(res.read_ok);
        EXPECT_EQ(res.value, v);
        EXPECT_EQ(res.bytes_written, 8u);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// 有符号整型 – 写入/读取往返
// ═══════════════════════════════════════════════════════════════════════════

TEST(BinaryRoundTrip, Int8)
{
    for (int8_t v : {int8_t(0), int8_t(1), int8_t(-1),
                     std::numeric_limits<int8_t>::max(),
                     std::numeric_limits<int8_t>::min()}) {
        auto res = roundtrip(v);
        EXPECT_TRUE(res.write_ok);
        EXPECT_TRUE(res.read_ok);
        EXPECT_EQ(res.value, v);
    }
}

TEST(BinaryRoundTrip, Int16)
{
    for (int16_t v : {int16_t(0), int16_t(-1), int16_t(0x1234),
                      std::numeric_limits<int16_t>::max(),
                      std::numeric_limits<int16_t>::min()}) {
        auto res = roundtrip(v);
        EXPECT_TRUE(res.write_ok);
        EXPECT_TRUE(res.read_ok);
        EXPECT_EQ(res.value, v);
    }
}

TEST(BinaryRoundTrip, Int32)
{
    for (int32_t v : {0, -1, 0x12345678,
                      std::numeric_limits<int32_t>::max(),
                      std::numeric_limits<int32_t>::min()}) {
        auto res = roundtrip(v);
        EXPECT_TRUE(res.write_ok);
        EXPECT_TRUE(res.read_ok);
        EXPECT_EQ(res.value, v);
        EXPECT_EQ(res.bytes_written, 4u);
    }
}

TEST(BinaryRoundTrip, Int64)
{
    for (int64_t v : {int64_t(0), int64_t(-1),
                      std::numeric_limits<int64_t>::max(),
                      std::numeric_limits<int64_t>::min()}) {
        auto res = roundtrip(v);
        EXPECT_TRUE(res.write_ok);
        EXPECT_TRUE(res.read_ok);
        EXPECT_EQ(res.value, v);
        EXPECT_EQ(res.bytes_written, 8u);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// 浮点型 – 写入/读取往返
// assemble_le 已改用 std::bit_cast，float/double LE 模式往返正确。
// ═══════════════════════════════════════════════════════════════════════════

TEST(BinaryRoundTrip, Float_LE)
{
    for (float v : {0.0f, 1.0f, -1.5f, 3.14159f,
                    std::numeric_limits<float>::max(),
                    std::numeric_limits<float>::lowest()}) {
        auto res = roundtrip(v);
        EXPECT_TRUE(res.write_ok);
        EXPECT_TRUE(res.read_ok);
        EXPECT_EQ(res.value, v);
        EXPECT_EQ(res.bytes_written, 4u);
    }
}

TEST(BinaryRoundTrip, Double_LE)
{
    for (double v : {0.0, 1.0, -1.5, 3.14159265358979,
                     std::numeric_limits<double>::max(),
                     std::numeric_limits<double>::lowest()}) {
        auto res = roundtrip(v);
        EXPECT_TRUE(res.write_ok);
        EXPECT_TRUE(res.read_ok);
        EXPECT_EQ(res.value, v);
        EXPECT_EQ(res.bytes_written, 8u);
    }
}

// 验证浮点写入字节数正确（即使值不正确）
TEST(BinaryRoundTrip, FloatWritesByteCount)
{
    char buf[4] = {};
    Writer w;
    w.buffer = buf;
    w.length  = sizeof(buf);
    EXPECT_TRUE(write(w, float{1.0f}));
    EXPECT_EQ(w.offset, 4u);
}

TEST(BinaryRoundTrip, DoubleWritesByteCount)
{
    char buf[8] = {};
    Writer w;
    w.buffer = buf;
    w.length  = sizeof(buf);
    EXPECT_TRUE(write(w, double{1.0}));
    EXPECT_EQ(w.offset, 8u);
}

// 记录当前（有 bug 的）实际行为：float 0.0f 的往返应特殊处理
// 0.0f 的位模式是全零，static_cast<float>(0) == 0.0f，因此 0.0f 例外可以通过
TEST(BinaryRoundTrip, FloatZeroRoundTrip)
{
    auto res = roundtrip(float{0.0f});
    EXPECT_TRUE(res.write_ok);
    EXPECT_TRUE(res.read_ok);
    EXPECT_EQ(res.value, 0.0f);  // 0.0f 位模式全零，static_cast 不影响结果
}

TEST(BinaryRoundTrip, DoubleZeroRoundTrip)
{
    auto res = roundtrip(double{0.0});
    EXPECT_TRUE(res.write_ok);
    EXPECT_TRUE(res.read_ok);
    EXPECT_EQ(res.value, 0.0);  // 0.0 位模式全零，static_cast 不影响结果
}

// ═══════════════════════════════════════════════════════════════════════════
// std::string – 写入/读取往返
// ═══════════════════════════════════════════════════════════════════════════

TEST(BinaryRoundTrip, StdStringEmpty)
{
    auto res = roundtrip(std::string{});
    EXPECT_TRUE(res.write_ok);
    EXPECT_TRUE(res.read_ok);
    EXPECT_EQ(res.value, "");
    EXPECT_EQ(res.bytes_written, 4u);  // 只有长度前缀
}

TEST(BinaryRoundTrip, StdStringNormal)
{
    const std::string s = "Hello, World!";
    auto res = roundtrip(s);
    EXPECT_TRUE(res.write_ok);
    EXPECT_TRUE(res.read_ok);
    EXPECT_EQ(res.value, s);
    EXPECT_EQ(res.bytes_written, 4u + s.size());
}

TEST(BinaryRoundTrip, StdStringWithSpaces)
{
    const std::string s = "foo bar  baz";
    auto res = roundtrip(s);
    EXPECT_TRUE(res.write_ok);
    EXPECT_TRUE(res.read_ok);
    EXPECT_EQ(res.value, s);
}

TEST(BinaryRoundTrip, StdStringLong)
{
    const std::string s(512, 'A');
    auto res = roundtrip(s, 600);
    EXPECT_TRUE(res.write_ok);
    EXPECT_TRUE(res.read_ok);
    EXPECT_EQ(res.value, s);
}

// ═══════════════════════════════════════════════════════════════════════════
// std::vector – 写入/读取往返
// ═══════════════════════════════════════════════════════════════════════════

TEST(BinaryRoundTrip, VectorEmpty)
{
    std::vector<uint32_t> v;
    auto res = roundtrip(v);
    EXPECT_TRUE(res.write_ok);
    EXPECT_TRUE(res.read_ok);
    EXPECT_TRUE(res.value.empty());
    EXPECT_EQ(res.bytes_written, 4u);  // 只有 count 前缀
}

TEST(BinaryRoundTrip, VectorUInt32)
{
    const std::vector<uint32_t> v{1u, 2u, 3u, 0xDEADBEEFu};
    auto res = roundtrip(v);
    EXPECT_TRUE(res.write_ok);
    EXPECT_TRUE(res.read_ok);
    EXPECT_EQ(res.value, v);
    EXPECT_EQ(res.bytes_written, 4u + 4u * sizeof(uint32_t));
}

TEST(BinaryRoundTrip, VectorInt32Negative)
{
    const std::vector<int32_t> v{0, -1, 100, -2147483648};
    auto res = roundtrip(v);
    EXPECT_TRUE(res.write_ok);
    EXPECT_TRUE(res.read_ok);
    EXPECT_EQ(res.value, v);
}

TEST(BinaryRoundTrip, VectorStrings)
{
    const std::vector<std::string> v{"foo", "", "bar", "hello world"};
    auto res = roundtrip(v);
    EXPECT_TRUE(res.write_ok);
    EXPECT_TRUE(res.read_ok);
    EXPECT_EQ(res.value, v);
}

TEST(BinaryRoundTrip, NestedVector)
{
    // vector<vector<int32_t>> 需要内层元素也有 read/write，递归验证
    const std::vector<std::vector<int32_t>> nested{{1, 2}, {3, 4, 5}, {}};
    auto res = roundtrip(nested);
    EXPECT_TRUE(res.write_ok);
    EXPECT_TRUE(res.read_ok);
    EXPECT_EQ(res.value, nested);
}

// ═══════════════════════════════════════════════════════════════════════════
// 自定义结构体 – 手动提供 size/write/read 重载
// ═══════════════════════════════════════════════════════════════════════════

namespace test_types {
struct Point {
    int32_t x = 0;
    int32_t y = 0;
    bool operator==(const Point&) const = default;
};

struct Packet {
    uint16_t    cmd  = 0;
    uint32_t    seq  = 0;
    std::string body;
    bool operator==(const Packet&) const = default;
};
} // namespace test_types

namespace yi::serialize::binary {

inline size_t size(const test_types::Point& p)
{
    return size(p.x) + size(p.y);
}
inline bool write(Writer& w, const test_types::Point& p)
{
    return write(w, p.x) && write(w, p.y);
}
inline bool read(Reader& r, test_types::Point& p)
{
    return read(r, p.x) && read(r, p.y);
}

inline size_t size(const test_types::Packet& pkt)
{
    return size(pkt.cmd) + size(pkt.seq) + size(pkt.body);
}
inline bool write(Writer& w, const test_types::Packet& pkt)
{
    return write(w, pkt.cmd) && write(w, pkt.seq) && write(w, pkt.body);
}
inline bool read(Reader& r, test_types::Packet& pkt)
{
    return read(r, pkt.cmd) && read(r, pkt.seq) && read(r, pkt.body);
}

} // namespace yi::serialize::binary

TEST(BinaryRoundTrip, CustomStruct_Point)
{
    test_types::Point p{3, -7};
    auto res = roundtrip(p);
    EXPECT_TRUE(res.write_ok);
    EXPECT_TRUE(res.read_ok);
    EXPECT_EQ(res.value, p);
    EXPECT_EQ(res.bytes_written, 8u);
}

TEST(BinaryRoundTrip, CustomStruct_Packet)
{
    test_types::Packet pkt{0x0101u, 42u, "payload data"};
    auto res = roundtrip(pkt);
    EXPECT_TRUE(res.write_ok);
    EXPECT_TRUE(res.read_ok);
    EXPECT_EQ(res.value, pkt);
    EXPECT_EQ(res.bytes_written, size(pkt));
}

TEST(BinaryRoundTrip, VectorOfStructs)
{
    const std::vector<test_types::Point> pts{{1, 2}, {-3, 4}, {0, 0}};
    auto res = roundtrip(pts);
    EXPECT_TRUE(res.write_ok);
    EXPECT_TRUE(res.read_ok);
    EXPECT_EQ(res.value, pts);
}

// ═══════════════════════════════════════════════════════════════════════════
// 缓冲区越界保护
// ═══════════════════════════════════════════════════════════════════════════

TEST(BinaryOverflow, WriteFailsOnTooSmallBuffer)
{
    char buf[2] = {};
    Writer w;
    w.buffer = buf;
    w.length = 2;
    // uint32_t 需要 4 字节，缓冲区只有 2 字节
    EXPECT_FALSE(write(w, uint32_t{42}));
    EXPECT_EQ(w.offset, 0u);  // 失败后偏移量不应前进
}

TEST(BinaryOverflow, WriteExactFit)
{
    char buf[4] = {};
    Writer w;
    w.buffer = buf;
    w.length = 4;
    EXPECT_TRUE(write(w, uint32_t{0x12345678u}));
    EXPECT_EQ(w.offset, 4u);
}

TEST(BinaryOverflow, WriteStringTooLong)
{
    // 缓冲区只有 5 字节：4 字节长度前缀 + 1 字节数据，不够放 "hello"(5字节)
    char buf[5] = {};
    Writer w;
    w.buffer = buf;
    w.length = 5;
    EXPECT_FALSE(write(w, std::string{"hello"}));
}

TEST(BinaryOverflow, WriteStringLengthPrefixFits)
{
    // 缓冲区只有 3 字节，连长度前缀（4 字节）都放不下
    char buf[3] = {};
    Writer w;
    w.buffer = buf;
    w.length = 3;
    EXPECT_FALSE(write(w, std::string{""}));
}

TEST(BinaryOverflow, ReadFailsOnTruncatedBuffer)
{
    char buf[4] = {0x01, 0x00, 0x00, 0x00};
    Reader r;
    r.buffer = buf;
    r.length = 2;  // 假装只有 2 字节可读
    uint32_t val = 0;
    EXPECT_FALSE(read(r, val));
}

TEST(BinaryOverflow, ReadStringDataTruncated)
{
    // 写入一个合法字符串，然后用更短的 length 读取
    char buf[32] = {};
    Writer w;
    w.buffer = buf;
    w.length = sizeof(buf);
    EXPECT_TRUE(write(w, std::string{"hello"}));  // 写入 9 字节

    // 只给 reader 7 字节：长度前缀(4) + 3 字节数据，读不完 "hello"(5字节)
    Reader r;
    r.buffer = buf;
    r.length = 7;
    std::string out;
    EXPECT_FALSE(read(r, out));
}

TEST(BinaryOverflow, MultipleWritesExhaustBuffer)
{
    char buf[6] = {};
    Writer w;
    w.buffer = buf;
    w.length = 6;

    EXPECT_TRUE(write(w, uint32_t{1}));   // 消耗 4 字节
    EXPECT_TRUE(write(w, uint16_t{2}));   // 消耗 2 字节，刚好用完
    EXPECT_FALSE(write(w, uint8_t{3}));   // 无剩余空间，应失败
}

// ═══════════════════════════════════════════════════════════════════════════
// 小端字节序验证（默认 is_big_endian = false）
// ═══════════════════════════════════════════════════════════════════════════

TEST(BinaryEndian, UInt16LittleEndian)
{
    char buf[2] = {};
    Writer w;
    w.buffer = buf;
    w.length  = 2;

    EXPECT_TRUE(write(w, uint16_t{0x0102}));
    // 小端：低字节在前
    EXPECT_EQ(byte_at(buf, 0), 0x02u);
    EXPECT_EQ(byte_at(buf, 1), 0x01u);
}

TEST(BinaryEndian, UInt32LittleEndian)
{
    char buf[4] = {};
    Writer w;
    w.buffer = buf;
    w.length  = 4;

    EXPECT_TRUE(write(w, uint32_t{0x01020304u}));
    EXPECT_EQ(byte_at(buf, 0), 0x04u);
    EXPECT_EQ(byte_at(buf, 1), 0x03u);
    EXPECT_EQ(byte_at(buf, 2), 0x02u);
    EXPECT_EQ(byte_at(buf, 3), 0x01u);
}

TEST(BinaryEndian, UInt64LittleEndian)
{
    char buf[8] = {};
    Writer w;
    w.buffer = buf;
    w.length  = 8;

    EXPECT_TRUE(write(w, uint64_t{0x0102030405060708ull}));
    EXPECT_EQ(byte_at(buf, 0), 0x08u);
    EXPECT_EQ(byte_at(buf, 1), 0x07u);
    EXPECT_EQ(byte_at(buf, 2), 0x06u);
    EXPECT_EQ(byte_at(buf, 3), 0x05u);
    EXPECT_EQ(byte_at(buf, 4), 0x04u);
    EXPECT_EQ(byte_at(buf, 5), 0x03u);
    EXPECT_EQ(byte_at(buf, 6), 0x02u);
    EXPECT_EQ(byte_at(buf, 7), 0x01u);
}

TEST(BinaryEndian, StringLengthPrefixLittleEndian)
{
    char buf[32] = {};
    Writer w;
    w.buffer = buf;
    w.length  = sizeof(buf);

    // 字符串 "ABCDE" 长度 5，长度前缀用 uint32_t LE 写入
    EXPECT_TRUE(write(w, std::string{"ABCDE"}));
    EXPECT_EQ(byte_at(buf, 0), 0x05u);  // LE：低字节先
    EXPECT_EQ(byte_at(buf, 1), 0x00u);
    EXPECT_EQ(byte_at(buf, 2), 0x00u);
    EXPECT_EQ(byte_at(buf, 3), 0x00u);
    EXPECT_EQ(buf[4], 'A');
    EXPECT_EQ(buf[8], 'E');
}

// ═══════════════════════════════════════════════════════════════════════════
// 特殊类型：vstr / fstr / vec<T,N> / std::array
// ═══════════════════════════════════════════════════════════════════════════

// ── vstr：自定义长度前缀的变长字符串 ────────────────────────────────────────

TEST(BinarySpecialTypes, VstrSize_1b)
{
    str1b s;
    s.assign("hi");
    EXPECT_EQ(size(s), sizeof(uint8_t) + 2u);  // 1字节前缀 + 2字节数据

    str1b empty;
    EXPECT_EQ(size(empty), sizeof(uint8_t));
}

TEST(BinarySpecialTypes, VstrSize_2b)
{
    str2b s;
    s.assign("hello");
    EXPECT_EQ(size(s), sizeof(uint16_t) + 5u);
}

TEST(BinarySpecialTypes, VstrRoundTrip_1b)
{
    str1b in;
    in.assign("binary");

    char buf[64] = {};
    Writer w;
    w.buffer = buf;
    w.length  = sizeof(buf);
    EXPECT_TRUE(write(w, in));
    EXPECT_EQ(w.offset, 1u + 6u);  // 1字节前缀 + 6字节数据

    str1b out;
    Reader r;
    r.buffer = buf;
    r.length  = w.offset;
    EXPECT_TRUE(read(r, out));
    EXPECT_EQ(std::string(out), std::string(in));
}

TEST(BinarySpecialTypes, VstrRoundTrip_2b)
{
    str2b in;
    in.assign("hello world");

    char buf[64] = {};
    Writer w;
    w.buffer = buf;
    w.length  = sizeof(buf);
    EXPECT_TRUE(write(w, in));
    EXPECT_EQ(w.offset, 2u + 11u);

    str2b out;
    Reader r;
    r.buffer = buf;
    r.length  = w.offset;
    EXPECT_TRUE(read(r, out));
    EXPECT_EQ(std::string(out), std::string(in));
}

// ── fstr：定长字符串（编译期大小） ──────────────────────────────────────────

TEST(BinarySpecialTypes, FstrSize)
{
    fstr<4>  f4;
    fstr<16> f16;
    EXPECT_EQ(size(f4),  4u);
    EXPECT_EQ(size(f16), 16u);
}

TEST(BinarySpecialTypes, FstrRoundTrip)
{
    fstr<8> in;
    in.assign("YICP");  // 4字节内容，剩余 4 字节填零

    char buf[8] = {};
    Writer w;
    w.buffer = buf;
    w.length  = sizeof(buf);
    EXPECT_TRUE(write(w, in));
    EXPECT_EQ(w.offset, 8u);

    // 验证剩余字节确实补零
    EXPECT_EQ(buf[4], '\0');
    EXPECT_EQ(buf[5], '\0');

    fstr<8> out;
    Reader r;
    r.buffer = buf;
    r.length  = w.offset;
    EXPECT_TRUE(read(r, out));
    EXPECT_EQ(out.substr(0, 4), "YICP");
}

TEST(BinarySpecialTypes, FstrExactFit)
{
    fstr<5> in;
    in.assign("hello");  // 恰好填满

    char buf[5] = {};
    Writer w;
    w.buffer = buf;
    w.length  = sizeof(buf);
    EXPECT_TRUE(write(w, in));
    EXPECT_EQ(w.offset, 5u);
    EXPECT_EQ(buf[4], 'o');
}

// ── vec<T, length_type>：自定义 count 前缀的变长数组 ───────────────────────

TEST(BinarySpecialTypes, VecCustomSize_1b)
{
    vec1b<uint32_t> v;
    v.push_back(1u);
    v.push_back(2u);
    v.push_back(3u);
    // 1字节 count 前缀 + 3 * 4字节数据
    EXPECT_EQ(size(v), sizeof(uint8_t) + 3u * sizeof(uint32_t));

    vec1b<uint32_t> empty;
    EXPECT_EQ(size(empty), sizeof(uint8_t));
}

TEST(BinarySpecialTypes, VecCustomRoundTrip_1b)
{
    vec1b<int32_t> in;
    in.push_back(10);
    in.push_back(-20);
    in.push_back(30);

    char buf[64] = {};
    Writer w;
    w.buffer = buf;
    w.length  = sizeof(buf);
    EXPECT_TRUE(write(w, in));
    EXPECT_EQ(w.offset, 1u + 3u * 4u);

    // count 前缀验证：1字节 LE，值为 3
    EXPECT_EQ(static_cast<uint8_t>(buf[0]), 3u);

    vec1b<int32_t> out;
    Reader r;
    r.buffer = buf;
    r.length  = w.offset;
    EXPECT_TRUE(read(r, out));
    EXPECT_EQ(out.size(), in.size());
    for (size_t i = 0; i < in.size(); ++i)
        EXPECT_EQ(out[i], in[i]);
}

TEST(BinarySpecialTypes, VecCustomRoundTrip_2b)
{
    vec2b<std::string> in;
    in.push_back("foo");
    in.push_back("bar");

    char buf[256] = {};
    Writer w;
    w.buffer = buf;
    w.length  = sizeof(buf);
    EXPECT_TRUE(write(w, in));

    vec2b<std::string> out;
    Reader r;
    r.buffer = buf;
    r.length  = w.offset;
    EXPECT_TRUE(read(r, out));
    EXPECT_EQ(out.size(), 2u);
    EXPECT_EQ(out[0], "foo");
    EXPECT_EQ(out[1], "bar");
}

// ── std::array：定长数组 ─────────────────────────────────────────────────────

TEST(BinarySpecialTypes, ArraySize)
{
    std::array<int32_t, 4> a = {1, 2, 3, 4};
    EXPECT_EQ(size(a), 4u * sizeof(int32_t));

    std::array<float, 3> af = {};
    EXPECT_EQ(size(af), 3u * sizeof(float));
}

TEST(BinarySpecialTypes, ArrayRoundTrip_Int32)
{
    std::array<int32_t, 4> in = {10, -20, 300, -4000};

    char buf[64] = {};
    Writer w;
    w.buffer = buf;
    w.length  = sizeof(buf);
    EXPECT_TRUE(write(w, in));
    EXPECT_EQ(w.offset, 4u * 4u);

    std::array<int32_t, 4> out = {};
    Reader r;
    r.buffer = buf;
    r.length  = w.offset;
    EXPECT_TRUE(read(r, out));
    EXPECT_EQ(out, in);
}

TEST(BinarySpecialTypes, ArrayRoundTrip_Float)
{
    std::array<float, 3> in = {1.0f, -2.5f, 3.14159f};

    char buf[32] = {};
    Writer w;
    w.buffer = buf;
    w.length  = sizeof(buf);
    EXPECT_TRUE(write(w, in));
    EXPECT_EQ(w.offset, 3u * 4u);

    std::array<float, 3> out = {};
    Reader r;
    r.buffer = buf;
    r.length  = w.offset;
    EXPECT_TRUE(read(r, out));
    EXPECT_EQ(out, in);
}

// ── 自定义类型与 vec / std::array 的组合 ────────────────────────────────────

// 注：size(item) 在容器模板内部仅依赖 ADL，对自定义类型只搜索其所在命名空间
// （test_types），而重载定义在 yi::serialize::binary，故容器级 size() 无法通过
// ADL 找到。write/read 因 Writer/Reader 参数提供 ADL 桥梁，不受此限制。
// 因此以下测试通过 w.offset 验证字节数，而非调用 size(container_of_custom)。

TEST(BinarySpecialTypes, VecCustomStruct_Point)
{
    // vec1b<Point>：1字节 count 前缀 + 每个 Point 占 8 字节
    vec1b<test_types::Point> in;
    in.push_back({1, 2});
    in.push_back({-3, 4});
    in.push_back({0, -100});

    // 验证单个元素 size() 正确（非模板上下文，ADL 可到达 yi::serialize::binary）
    EXPECT_EQ(size(test_types::Point{}), 8u);

    char buf[64] = {};
    Writer w;
    w.buffer = buf;
    w.length  = sizeof(buf);
    EXPECT_TRUE(write(w, in));
    EXPECT_EQ(w.offset, 1u + 3u * 8u);  // 1字节前缀 + 3 * (4+4)

    // count 前缀验证：1字节 LE，值为 3
    EXPECT_EQ(static_cast<uint8_t>(buf[0]), 3u);

    vec1b<test_types::Point> out;
    Reader r;
    r.buffer = buf;
    r.length  = w.offset;
    EXPECT_TRUE(read(r, out));
    ASSERT_EQ(out.size(), in.size());
    for (size_t i = 0; i < in.size(); ++i)
        EXPECT_EQ(out[i], in[i]) << "mismatch at index " << i;
}

TEST(BinarySpecialTypes, VecCustomStruct_Empty)
{
    // 空的 vec2b<Point>：只写 2 字节 count 前缀，值为 0
    vec2b<test_types::Point> empty;

    char buf[4] = {};
    Writer w;
    w.buffer = buf;
    w.length  = sizeof(buf);
    EXPECT_TRUE(write(w, empty));
    EXPECT_EQ(w.offset, 2u);  // 仅 uint16_t count 前缀

    vec2b<test_types::Point> out;
    Reader r;
    r.buffer = buf;
    r.length  = w.offset;
    EXPECT_TRUE(read(r, out));
    EXPECT_TRUE(out.empty());
}

TEST(BinarySpecialTypes, ArrayCustomStruct_Point)
{
    // std::array<Point, 3>：无 count 前缀，总字节 = 3 * 8
    std::array<test_types::Point, 3> in = {{{1, 2}, {-3, 4}, {0, 0}}};

    char buf[64] = {};
    Writer w;
    w.buffer = buf;
    w.length  = sizeof(buf);
    EXPECT_TRUE(write(w, in));
    EXPECT_EQ(w.offset, 3u * 8u);

    std::array<test_types::Point, 3> out = {};
    Reader r;
    r.buffer = buf;
    r.length  = w.offset;
    EXPECT_TRUE(read(r, out));
    EXPECT_EQ(out, in);
}

TEST(BinarySpecialTypes, ArrayCustomStruct_Packet)
{
    // std::array<Packet, 2>：无 count 前缀，每个 Packet 大小可变（含字符串）
    std::array<test_types::Packet, 2> in = {{{0x01u, 1u, "hello"}, {0x02u, 2u, ""}}};

    // size() 对单个 Packet 可用（非模板上下文）
    const size_t expected_bytes = size(in[0]) + size(in[1]);

    char buf[256] = {};
    Writer w;
    w.buffer = buf;
    w.length  = sizeof(buf);
    EXPECT_TRUE(write(w, in));
    EXPECT_EQ(w.offset, expected_bytes);

    std::array<test_types::Packet, 2> out = {};
    Reader r;
    r.buffer = buf;
    r.length  = w.offset;
    EXPECT_TRUE(read(r, out));
    EXPECT_EQ(out, in);
}

// ─── 大端字节序验证 ────────────────────────────────────────────────────────

TEST(BinaryBigEndian, UInt16BigEndian)
{
    char buf[2] = {};
    WriterT<true> w;
    w.buffer = buf;
    w.length  = 2;

    EXPECT_TRUE(write(w, uint16_t{0x0102}));
    // 大端：高字节在前
    EXPECT_EQ(static_cast<uint8_t>(buf[0]), 0x01u);
    EXPECT_EQ(static_cast<uint8_t>(buf[1]), 0x02u);
}

TEST(BinaryBigEndian, UInt32BigEndian)
{
    char buf[4] = {};
    WriterT<true> w;
    w.buffer = buf;
    w.length  = 4;

    EXPECT_TRUE(write(w, uint32_t{0x01020304u}));
    EXPECT_EQ(static_cast<uint8_t>(buf[0]), 0x01u);
    EXPECT_EQ(static_cast<uint8_t>(buf[1]), 0x02u);
    EXPECT_EQ(static_cast<uint8_t>(buf[2]), 0x03u);
    EXPECT_EQ(static_cast<uint8_t>(buf[3]), 0x04u);
}

TEST(BinaryBigEndian, RoundTripBigEndian)
{
    char buf[32] = {};
    WriterT<true> w;
    w.buffer = buf;
    w.length  = sizeof(buf);
    EXPECT_TRUE(write(w, uint64_t{0xDEADBEEFCAFEBABEull}));
    EXPECT_TRUE(write(w, int32_t{-42}));

    ReaderT<true> r;
    r.buffer = buf;
    r.length  = w.offset;

    uint64_t v1 = 0;
    int32_t  v2 = 0;
    EXPECT_TRUE(read(r, v1));
    EXPECT_TRUE(read(r, v2));
    EXPECT_EQ(v1, 0xDEADBEEFCAFEBABEull);
    EXPECT_EQ(v2, -42);
}

// ═══════════════════════════════════════════════════════════════════════════
// offset 追踪验证
// ═══════════════════════════════════════════════════════════════════════════

TEST(BinaryOffset, OffsetAdvancesCorrectly)
{
    char buf[32] = {};
    Writer w;
    w.buffer = buf;
    w.length  = sizeof(buf);

    EXPECT_EQ(w.offset, 0u);
    write(w, uint8_t{1});
    EXPECT_EQ(w.offset, 1u);
    write(w, uint32_t{2});
    EXPECT_EQ(w.offset, 5u);
    write(w, uint64_t{3});
    EXPECT_EQ(w.offset, 13u);
}

TEST(BinaryOffset, ReaderOffsetAdvancesCorrectly)
{
    char buf[13] = {};
    Writer w;
    w.buffer = buf;
    w.length  = sizeof(buf);
    write(w, uint8_t{10});
    write(w, uint32_t{20});
    write(w, uint64_t{30});

    Reader r;
    r.buffer = buf;
    r.length  = w.offset;

    uint8_t  v1 = 0;
    uint32_t v2 = 0;
    uint64_t v3 = 0;

    EXPECT_EQ(r.offset, 0u);
    EXPECT_TRUE(read(r, v1)); EXPECT_EQ(v1, 10u);  EXPECT_EQ(r.offset, 1u);
    EXPECT_TRUE(read(r, v2)); EXPECT_EQ(v2, 20u);  EXPECT_EQ(r.offset, 5u);
    EXPECT_TRUE(read(r, v3)); EXPECT_EQ(v3, 30u);  EXPECT_EQ(r.offset, 13u);
}
