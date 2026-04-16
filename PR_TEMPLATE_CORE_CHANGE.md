# yi_core 变更 PR 模板

## 1. 改动摘要

- 改了什么：
- 为什么改：
- 影响范围：

## 2. 结构与规则检查

- [ ] 目录结构符合 `core/CORE_STRUCTURE.md`
- [ ] 对外头文件仅放在 `include/yi/...`
- [ ] 跨服务共享类型放在 `include/yi/core/common`
- [ ] CMake 目标名为 `yi_core`
- [ ] 导出 `YiCoreTargets`，并提供 `YiCoreConfig.cmake` / `YiCoreConfigVersion.cmake`
- [ ] `project(yi_core VERSION x.y.z)` 版本号与发布计划一致

## 3. 兼容性评估

- [ ] 无 breaking change
- [ ] 有 breaking change（需填写迁移说明）

### 迁移说明（仅 breaking change 必填）

- 受影响 API：
- 推荐替代 API：
- 迁移步骤：
- 回滚方案：

## 4. 测试与验证

- 本地构建：
  - [ ] `cmake -S . -B build`
  - [ ] `cmake --build build`
- 单元测试：
  - [ ] 已新增/更新测试
  - [ ] 正常路径覆盖
  - [ ] 边界路径覆盖

## 5. 发布信息

- 计划版本：`0.0.1` / `x.y.z`
- Tag：
- Changelog 条目：
- 下游仓库影响（`modules` / `user-service`）：

## 6. 审阅重点（给 Reviewer）

- API 设计是否可测试且最小公开
- 是否引入不必要依赖
- 是否遵守 `yi_core` 低频发布与兼容性策略
