# yi-core

版本：0.0.2

`yi-core` 是 yi-workspace 的 L0 核心框架库，提供序列化抽象、基础类型与适配器接口。
更新频率极低；破坏性变更需 major 版本并提前通知消费方。

## 功能模块

| 模块 | 头文件入口 | 说明 |
| --- | --- | --- |
| 基础类型 | `<yi/core/common/bytes.h>` | `Bytes`、`Base64`、`ByteBuffer` |
| 二进制序列化 | `<yi/core/serialize/binary/binary.h>` | 小端/大端、标量/string/vector/自定义结构 |
| JSON 序列化 | `<yi/core/serialize/json/json.h>` | 基于 RapidJSON，支持 JSONC 注释 |
| 协程接口（预留） | `<yi/core/coro/task.h>` | C++20 协程 Task 接口（待实现） |
| 适配器（骨架） | `<yi/adapter/*/...>` | cpprestsdk / kafka / libuv / mysql / nacos / sqlserver |

## 快速开始

```bash
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . -j$(nproc)
ctest --output-on-failure
```

## 在下游项目中引入

```cmake
FetchContent_Declare(yi_core
    GIT_REPOSITORY https://github.com/yiyongxin/yi-core.git
    GIT_TAG v0.0.2
    SOURCE_DIR ${LOCAL_ROOT}/framework/yi-core   # 工作台本地覆盖
)
FetchContent_MakeAvailable(yi_core)
target_link_libraries(my_target PRIVATE yi::core)
```

## 目录结构

```
yi-core/
├── include/yi/
│   ├── core/
│   │   ├── common/         基础类型（Bytes、Base64、ByteBuffer）
│   │   ├── coro/           协程接口（预留）
│   │   ├── md5/            MD5（待实现）
│   │   └── serialize/
│   │       ├── binary/     二进制序列化
│   │       └── json/       JSON 序列化
│   └── adapter/            外部库适配器接口
├── src/                    编译单元（当前为占位符）
├── cmake/                  CMake 导出模板
├── tests/                  GTest 单元测试
├── example/                使用示例
└── docs/
    ├── CORE_STRUCTURE.md   仓库约定与发布策略
    └── PR_TEMPLATE_CORE_CHANGE.md
```

## 命名约定

- C++ 命名空间：`yi::common::`（基础类型）、`yi::serialize::binary::`、`yi::serialize::json::`
- CMake 目标：`yi::core`
- 头文件路径：`<yi/core/模块/头文件.h>`

参阅 `docs/CORE_STRUCTURE.md` 了解详细约定与发布流程。
