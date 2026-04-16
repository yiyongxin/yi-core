# yi_core 仓库结构与约定

本文档描述 `yi_core`（核心框架）在仓库层面的目录结构、CMake 约定、版本策略与发布流程，供基础团队与依赖方参考。

## 目标

- 提供一致的项目布局，便于 `find_package(yi_core)` 集成。
- 明确导出、安装与版本策略，降低跨团队集成成本。

## 目录布局（必备）

```
yi_core/                       # Git 仓库根
├── include/yi/                 # 对外公开头（必需）
│   └── core/                   # 公共类型/协议
├── src/                        # 源码实现（与 include 镜像）
├── cmake/                      # 导出与安装模板（YiCoreConfig.cmake.in）
├── tests/                      # 单元测试
├── examples/                   # 使用示例
├── CMakeLists.txt
└── README.md
```

命名说明：对外头文件以 `include/yi/...` 为根，项目名与 CMake `project()` 使用 `yi_core`。

## CMake 与导出约定

- CMake 项目例子（最小）：

```cmake
cmake_minimum_required(VERSION 3.24)
project(yi_core VERSION 0.0.1)

add_library(yi_core STATIC
    src/placeholder.cpp
)

target_include_directories(yi_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

install(TARGETS yi_core EXPORT YiCoreTargets)
install(EXPORT YiCoreTargets FILE YiCoreTargets.cmake DESTINATION lib/cmake/yi_core)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/YiCoreConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
  cmake/YiCoreConfig.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/YiCoreConfig.cmake"
  INSTALL_DESTINATION lib/cmake/yi_core
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/YiCoreConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/YiCoreConfigVersion.cmake"
  DESTINATION lib/cmake/yi_core
)
```

## 版本与发布策略

- 初始版本：`0.0.1`。
- `yi_core` 属于基础框架，采用低频发布策略：重要且兼容性破坏的改动需通过 major 版本并提前通知消费方。
- 发布流程：在核心仓库中由基础团队负责 tag、发布与变更日志维护。

## 本地联调（开发者工作流）

- 当在业务仓库（如 `user-service`）同时调试 `yi_core` 时，使用 CMake `FetchContent` 并设置 `SOURCE_DIR` 指向本地同级目录以覆盖远程依赖，示例见 `user-service/CMakeLists.txt`。

## 测试与示例

- `tests/` 提供单元测试，建议使用 Catch2/GoogleTest（视团队惯例）。
- `examples/` 给出常见场景（序列化、类型定义、RPC 适配器示例）。

## 发布注意事项

- 在变更公共头文件前，先与基础团队沟通并更新 `CHANGELOG.md`。
- 对外公开的公共类型应放在 `include/yi/core/common` 并保持向后兼容。

---

如需扩展的约定（例如 ABI 稳定性策略、CI 发布脚本），请把补充条目提交到本文件并由基础团队审阅。