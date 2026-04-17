#pragma once

// yi::Task<T> — C++20 协程任务接口（预留，待 v0.1.0 实现）
//
// 占位说明：
//   工作台设计（L0）要求 yi-core 提供协程与调度器抽象。
//   本头文件定义接口轮廓，实现将在 devel/v0.1.0 分支完成。
//
// 预期用法：
//   yi::Task<int> fetch_value() {
//       co_return 42;
//   }

#include <coroutine>
#include <exception>
#include <optional>

namespace yi
{

template<typename T = void>
struct Task;

// void 特化声明（完整实现待补充）
template<>
struct Task<void>
{
    struct promise_type {
        Task get_return_object() { return {}; }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };
};

} // namespace yi
