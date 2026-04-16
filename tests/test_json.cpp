#include <gtest/gtest.h>
#include "serialize/json/json.h"

#include <chrono>
#include <cstdint>
#include <list>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

using namespace yi::serialize::json;

// ── 辅助：编码后再解析，返回是否成功及结果值 ──────────────────────────────

template <typename T>
std::pair<bool, T> roundtrip(const T& in)
{
    auto [enc_ok, json] = encode(in);
    if (!enc_ok) return {false, {}};

    T out{};
    bool dec_ok = parse(out, json);
    return {dec_ok, out};
}

// ═══════════════════════════════════════════════════════════════════════════
// 基本类型 – 编码/解析往返
// ═══════════════════════════════════════════════════════════════════════════

TEST(JsonPrimitive, Bool)
{
    {
        auto [ok, v] = roundtrip(true);
        EXPECT_TRUE(ok);
        EXPECT_TRUE(v);
    }
    {
        auto [ok, v] = roundtrip(false);
        EXPECT_TRUE(ok);
        EXPECT_FALSE(v);
    }
}

TEST(JsonPrimitive, Int32)
{
    for (int32_t v : {int32_t(0), int32_t(1), int32_t(-1), int32_t(42),
                      int32_t(-100), std::numeric_limits<int32_t>::max(),
                      std::numeric_limits<int32_t>::min()}) {
        auto [ok, out] = roundtrip(v);
        EXPECT_TRUE(ok) << "roundtrip failed for v=" << v;
        EXPECT_EQ(out, v);
    }
}

TEST(JsonPrimitive, Int64)
{
    for (int64_t v : {int64_t(0), int64_t(-1),
                      std::numeric_limits<int64_t>::max(),
                      std::numeric_limits<int64_t>::min()}) {
        auto [ok, out] = roundtrip(v);
        EXPECT_TRUE(ok);
        EXPECT_EQ(out, v);
    }
}

TEST(JsonPrimitive, UInt32)
{
    for (uint32_t v : {0u, 1u, 42u, std::numeric_limits<uint32_t>::max()}) {
        auto [ok, out] = roundtrip(v);
        EXPECT_TRUE(ok) << "roundtrip failed for v=" << v;
        EXPECT_EQ(out, v);
    }
}

TEST(JsonPrimitive, UInt64)
{
    for (uint64_t v : {uint64_t(0), uint64_t(1),
                       std::numeric_limits<uint64_t>::max()}) {
        auto [ok, out] = roundtrip(v);
        EXPECT_TRUE(ok);
        EXPECT_EQ(out, v);
    }
}

TEST(JsonPrimitive, Double)
{
    for (double v : {0.0, 1.0, -1.5, 3.14159265358979}) {
        auto [ok, out] = roundtrip(v);
        EXPECT_TRUE(ok);
        EXPECT_DOUBLE_EQ(out, v);
    }
}

TEST(JsonPrimitive, Float)
{
    for (float v : {0.0f, 1.0f, -1.5f, 3.14f}) {
        auto [ok, out] = roundtrip(v);
        EXPECT_TRUE(ok);
        EXPECT_FLOAT_EQ(out, v);
    }
}

TEST(JsonPrimitive, String)
{
    for (const std::string& s : {"", "hello", "hello world", "with \"quotes\""}) {
        auto [ok, out] = roundtrip(s);
        EXPECT_TRUE(ok) << "failed for s=\"" << s << "\"";
        EXPECT_EQ(out, s);
    }
}

TEST(JsonPrimitive, ChronoSeconds)
{
    using namespace std::chrono_literals;
    for (auto d : {0s, 1s, 3600s, -1s}) {
        auto [ok, out] = roundtrip(d);
        EXPECT_TRUE(ok);
        EXPECT_EQ(out, d);
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// encode 产生的 JSON 内容验证
// ═══════════════════════════════════════════════════════════════════════════

TEST(JsonEncode, BoolValue)
{
    auto [ok, json] = encode(true);
    EXPECT_TRUE(ok);
    EXPECT_EQ(json, "true");

    auto [ok2, json2] = encode(false);
    EXPECT_TRUE(ok2);
    EXPECT_EQ(json2, "false");
}

TEST(JsonEncode, IntValue)
{
    auto [ok, json] = encode(int32_t{42});
    EXPECT_TRUE(ok);
    EXPECT_EQ(json, "42");
}

TEST(JsonEncode, NegativeInt)
{
    auto [ok, json] = encode(int32_t{-7});
    EXPECT_TRUE(ok);
    EXPECT_EQ(json, "-7");
}

TEST(JsonEncode, StringValue)
{
    auto [ok, json] = encode(std::string{"hello"});
    EXPECT_TRUE(ok);
    EXPECT_EQ(json, "\"hello\"");
}

// ═══════════════════════════════════════════════════════════════════════════
// parse 错误情况
// ═══════════════════════════════════════════════════════════════════════════

TEST(JsonParseError, InvalidJson)
{
    int32_t v = 0;
    EXPECT_FALSE(parse(v, "not_json"));
    EXPECT_FALSE(parse(v, "{broken"));
    EXPECT_FALSE(parse(v, ""));
}

TEST(JsonParseError, TypeMismatch_IntFromString)
{
    int32_t v = 0;
    EXPECT_FALSE(parse(v, "\"hello\""));  // 字符串 != 整数
}

TEST(JsonParseError, TypeMismatch_BoolFromInt)
{
    bool v = false;
    EXPECT_FALSE(parse(v, "1"));  // 数字 != bool
}

TEST(JsonParseError, TypeMismatch_StringFromInt)
{
    std::string v;
    EXPECT_FALSE(parse(v, "42"));  // 数字 != 字符串
}

// ═══════════════════════════════════════════════════════════════════════════
// 容器类型 – vector
// ═══════════════════════════════════════════════════════════════════════════

TEST(JsonVector, EmptyInt)
{
    std::vector<int32_t> v;
    auto [ok, out] = roundtrip(v);
    EXPECT_TRUE(ok);
    EXPECT_TRUE(out.empty());
}

TEST(JsonVector, Ints)
{
    const std::vector<int32_t> v{1, 2, 3, -4, 0};
    auto [ok, out] = roundtrip(v);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out, v);
}

TEST(JsonVector, Strings)
{
    const std::vector<std::string> v{"foo", "bar", "", "baz"};
    auto [ok, out] = roundtrip(v);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out, v);
}

// std::vector<bool> 是特化容器（代理引用），不支持框架的 emplace_back() 写入模式，
// 使用 vector<uint8_t> 替代测试布尔序列
TEST(JsonVector, BoolsAsUInt8)
{
    const std::vector<uint8_t> v{1u, 0u, 1u, 1u, 0u};
    auto [ok, out] = roundtrip(v);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out, v);
}

TEST(JsonVector, Nested)
{
    auto [ok, json] = encode(std::vector<int32_t>{10, 20, 30});
    EXPECT_TRUE(ok);
    EXPECT_EQ(json, "[10,20,30]");
}

// ═══════════════════════════════════════════════════════════════════════════
// 容器类型 – list
// ═══════════════════════════════════════════════════════════════════════════

TEST(JsonList, Strings)
{
    const std::list<std::string> lst{"a", "b", "c"};
    auto [ok, out] = roundtrip(lst);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out, lst);
}

TEST(JsonList, Empty)
{
    std::list<int32_t> lst;
    auto [ok, out] = roundtrip(lst);
    EXPECT_TRUE(ok);
    EXPECT_TRUE(out.empty());
}

// ═══════════════════════════════════════════════════════════════════════════
// 容器类型 – set
// ═══════════════════════════════════════════════════════════════════════════

TEST(JsonSet, Ints)
{
    const std::set<int32_t> s{1, 2, 3, 4, 5};
    auto [ok, out] = roundtrip(s);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out, s);
}

TEST(JsonSet, Strings)
{
    const std::set<std::string> s{"alpha", "beta", "gamma"};
    auto [ok, out] = roundtrip(s);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out, s);
}

// ═══════════════════════════════════════════════════════════════════════════
// 容器类型 – optional
// ═══════════════════════════════════════════════════════════════════════════

TEST(JsonOptional, PresentString)
{
    std::optional<std::string> opt = "hello";
    auto [enc_ok, json] = encode(opt);
    EXPECT_TRUE(enc_ok);
    // 有值时直接写出内部值
    EXPECT_EQ(json, "\"hello\"");

    std::optional<std::string> out;
    EXPECT_TRUE(parse(out, json));
    EXPECT_TRUE(out.has_value());
    EXPECT_EQ(*out, "hello");
}

TEST(JsonOptional, PresentInt)
{
    std::optional<int32_t> opt = 42;
    auto [ok, out] = roundtrip(opt);
    EXPECT_TRUE(ok);
    EXPECT_TRUE(out.has_value());
    EXPECT_EQ(*out, 42);
}

TEST(JsonOptional, EmptyWritesNothing)
{
    std::optional<std::string> opt;
    auto [enc_ok, json] = encode(opt);
    // 空 optional 不写任何值，buffer 为空 → 非合法 JSON → encode 仍返回 ok
    // 但 json 是空字符串（无法 parse 回来）
    EXPECT_TRUE(enc_ok);
    EXPECT_EQ(json, "");
}

// ═══════════════════════════════════════════════════════════════════════════
// 容器类型 – map
// ═══════════════════════════════════════════════════════════════════════════

TEST(JsonMap, StringKeyInt)
{
    const std::map<std::string, int32_t> m{{"a", 1}, {"b", 2}, {"c", 3}};
    auto [ok, out] = roundtrip(m);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out, m);
}

TEST(JsonMap, StringKeyString)
{
    const std::map<std::string, std::string> m{{"key1", "val1"}, {"key2", "val2"}};
    auto [ok, out] = roundtrip(m);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out, m);
}

TEST(JsonMap, Int64Key)
{
    const std::map<int64_t, std::string> m{{1, "one"}, {2, "two"}, {-1, "neg"}};
    auto [ok, out] = roundtrip(m);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out, m);
}

TEST(JsonMap, EmptyMap)
{
    std::map<std::string, int32_t> m;
    auto [ok, out] = roundtrip(m);
    EXPECT_TRUE(ok);
    EXPECT_TRUE(out.empty());
}

// ═══════════════════════════════════════════════════════════════════════════
// 自定义对象 – 通过 parse / encode 成员函数
// ═══════════════════════════════════════════════════════════════════════════

namespace test_json_types {

struct Point {
    int32_t     x = 0;
    int32_t     y = 0;
    bool operator==(const Point&) const = default;

    void parse(Reader& reader)
    {
        read(reader, "x", x);
        read(reader, "y", y);
    }
    void encode(Writer& writer) const
    {
        write(writer, "x", x);
        write(writer, "y", y);
    }
};

struct Person {
    std::string         name;
    int32_t             age  = 0;
    std::vector<int32_t> scores;
    bool operator==(const Person&) const = default;

    void parse(Reader& reader)
    {
        read(reader, "name",   name);
        read(reader, "age",    age);
        read(reader, "scores", scores);
    }
    void encode(Writer& writer) const
    {
        write(writer, "name",   name);
        write(writer, "age",    age);
        write(writer, "scores", scores);
    }
};

} // namespace test_json_types

TEST(JsonObject, SimplePoint)
{
    test_json_types::Point p{3, -7};
    auto [ok, out] = roundtrip(p);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out, p);
}

TEST(JsonObject, PointEncodeFormat)
{
    test_json_types::Point p{1, 2};
    auto [ok, json] = encode(p);
    EXPECT_TRUE(ok);
    // rapidjson 默认不加空格
    EXPECT_EQ(json, "{\"x\":1,\"y\":2}");
}

TEST(JsonObject, PointOrigin)
{
    test_json_types::Point p{0, 0};
    auto [ok, out] = roundtrip(p);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out.x, 0);
    EXPECT_EQ(out.y, 0);
}

TEST(JsonObject, PersonWithVector)
{
    test_json_types::Person person{"Alice", 30, {95, 87, 100}};
    auto [ok, out] = roundtrip(person);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out, person);
}

TEST(JsonObject, ParseFromLiteral)
{
    const std::string json = R"({"x":10,"y":-5})";
    test_json_types::Point p;
    EXPECT_TRUE(parse(p, json));
    EXPECT_EQ(p.x, 10);
    EXPECT_EQ(p.y, -5);
}

TEST(JsonObject, MissingFieldKeepsDefault)
{
    // JSON 中缺少 "y" 字段，y 应保持默认值 0
    const std::string json = R"({"x":99})";
    test_json_types::Point p;
    p.y = 0;  // 确认初始值
    EXPECT_TRUE(parse(p, json));
    EXPECT_EQ(p.x, 99);
    EXPECT_EQ(p.y, 0);
}

// ═══════════════════════════════════════════════════════════════════════════
// JSONC – 注释剥离
// ═══════════════════════════════════════════════════════════════════════════

TEST(JsoncCommentRemoval, SingleLineComment)
{
    const std::string jsonc = R"(// 这是注释
42)";
    std::string cleaned = remove_comments(jsonc);
    // 注释行被移除，剩下 "42"（前面可能有换行）
    int32_t v = 0;
    EXPECT_TRUE(parse(v, cleaned));
    EXPECT_EQ(v, 42);
}

TEST(JsoncCommentRemoval, BlockComment)
{
    const std::string jsonc = "/* 块注释 */ 99";
    std::string cleaned = remove_comments(jsonc);
    int32_t v = 0;
    EXPECT_TRUE(parse(v, cleaned));
    EXPECT_EQ(v, 99);
}

TEST(JsoncCommentRemoval, InlineComment)
{
    const std::string jsonc = R"({
        "x": 1, // x 坐标
        "y": 2  /* y 坐标 */
    })";
    test_json_types::Point p;
    EXPECT_TRUE(parse_jsonc(p, jsonc));
    EXPECT_EQ(p.x, 1);
    EXPECT_EQ(p.y, 2);
}

TEST(JsoncCommentRemoval, NoComments)
{
    // 无注释时原样输出
    const std::string json = R"({"x":5,"y":6})";
    EXPECT_EQ(remove_comments(json), json);
}

TEST(JsoncCommentRemoval, CommentInsideStringIsPreserved)
{
    // 字符串内的 // 不应被当作注释
    const std::string jsonc = R"("http://example.com")";
    const std::string cleaned = remove_comments(jsonc);
    std::string v;
    EXPECT_TRUE(parse(v, cleaned));
    EXPECT_EQ(v, "http://example.com");
}

TEST(JsoncParseJsonc, FullExample)
{
    const std::string jsonc = R"({
        // 姓名字段
        "name": "Bob",
        "age": 25,  /* 年龄 */
        "scores": [80, 90, 70]
    })";
    test_json_types::Person p;
    EXPECT_TRUE(parse_jsonc(p, jsonc));
    EXPECT_EQ(p.name, "Bob");
    EXPECT_EQ(p.age, 25);
    EXPECT_EQ(p.scores, (std::vector<int32_t>{80, 90, 70}));
}

// ═══════════════════════════════════════════════════════════════════════════
// 嵌套结构
// ═══════════════════════════════════════════════════════════════════════════

namespace test_json_types {

struct Team {
    std::string             name;
    std::vector<Person>     members;
    bool operator==(const Team&) const = default;

    void parse(Reader& reader)
    {
        read(reader, "name",    name);
        read(reader, "members", members);
    }
    void encode(Writer& writer) const
    {
        write(writer, "name",    name);
        write(writer, "members", members);
    }
};

} // namespace test_json_types

// read/write for Person 通过 parse/encode 成员函数自动分发，无需额外重载

TEST(JsonNested, TeamWithMembers)
{
    test_json_types::Team team{
        "Dev",
        {
            {"Alice", 30, {95, 87}},
            {"Bob",   25, {80}}
        }
    };
    auto [ok, out] = roundtrip(team);
    EXPECT_TRUE(ok);
    EXPECT_EQ(out, team);
}
