#pragma once

// yi::Task<T> — 基于 stdexec exec::task<T> 的协程任务类型
//
// 约束（强制执行）：
//   协程只在发起的线程上运行。在协程体内部禁止使用：
//     - stdexec::on(other_scheduler, ...)
//     - stdexec::transfer(sender, other_scheduler)
//   跨线程通信请使用 yi::coro::spawn_on / yi::coro::submit。
//
// 用法示例：
//   yi::Task<int> compute(int x) {
//       co_return x * 2;
//   }
//
//   yi::Task<void> pipeline() {
//       int val = co_await compute(21);
//       co_return;
//   }

#include <exec/task.hpp>

namespace yi {

template<typename T = void>
using Task = exec::task<T>;

} // namespace yi
