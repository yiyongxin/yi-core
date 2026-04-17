#pragma once

// yi 协程模块统一入口
//
// 包含此头文件即可使用完整的协程与跨线程桥接功能：
//   - yi::Task<T>               协程任务类型
//   - yi::coro::ThreadExecutor  单线程事件循环
//   - yi::coro::spawn_on()      跨线程投递协程，返回 future<T>
//   - yi::coro::submit()        跨线程投递普通函数，返回 future<T>

#include <yi/core/coro/executor.h>
#include <yi/core/coro/task.h>
#include <yi/core/coro/thread_bridge.h>
