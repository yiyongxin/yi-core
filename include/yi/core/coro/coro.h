#pragma once

// 协程模块统一入口
//
// 包含本头文件即可使用协程基础设施的全部功能：
//   - yi::Task<T>                         协程任务类型
//   - yi::coro::ThreadContext             stdexec run_loop 执行上下文
//   - yi::coro::spawn_on / submit         协程与普通线程的桥接工具
//   - yi::coro::transfer_to               协程内跨上下文迁移

#include "context/thread_context.h" // IWYU pragma: export
#include "task.h"                   // IWYU pragma: export
#include "thread_bridge.h"          // IWYU pragma: export
