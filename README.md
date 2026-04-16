# yi_core (核心库示例)

这是 `yi_core` 的最小示例实现，展示核心库的目录约定、CMake 导出与本地联调方式。

目录结构（示例）：

- `include/yi/`：公共头文件（如 `include/yi/core/common`、`include/yi/core/serialize`）
- `src/`：实现代码
- `cmake/`：CMake 导出与安装模板（如 `YiCoreConfig.cmake.in`）
- `tests/`：单元测试
- `examples/`：使用示例

版本：0.0.1

快速开始：

```bash
mkdir -p build && cd build
cmake ..
cmake --build .
```

请参阅 `CORE_STRUCTURE.md` 了解详细约定和发布流程。
