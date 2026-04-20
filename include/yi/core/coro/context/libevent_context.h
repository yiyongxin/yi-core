#pragma once

// libevent 事件循环执行上下文
//
// 依赖：libevent（CMake: find_package(Libevent) 或 pkg_check_modules(LIBEVENT libevent)）

#include "detail/posting_scheduler.h"

#include <event2/event.h>

#include <functional>
#include <mutex>
#include <queue>

namespace yi::coro {

/// @brief 基于 libevent event_base 的执行上下文。
///
/// 将 libevent 事件循环封装为 yi::coro 标准执行上下文，提供与 ThreadContext
/// 完全一致的 scheduler() / post() / run() / finish() 接口。
///
/// 额外能力：
///   - base() 暴露底层 event_base*，供业务代码直接注册 libevent 事件
///     （socket、信号、定时器等），与 post() 投递的任务共存于同一事件循环。
///
/// 线程模型：
///   - run() 调用线程即"归属线程"，所有通过 post() 投递的任务在此线程执行。
///   - post() 本身线程安全；内部使用 event_base_once + 0 超时实现跨线程唤醒，
///     这是 libevent 官方推荐的线程安全投递手法。
///
/// 生命周期注意事项：
///   - 析构时自动调用 event_base_free；析构前须确保 run() 已返回。
///   - 在 base() 上注册的事件须在 finish() 前手动删除，否则 run() 不会退出。
///
/// 典型用法：
/// @code
///   LibeventContext ctx;
///   std::thread t([&]{ ctx.run(); });
///
///   // 注册一个 libevent 定时器
///   event* ev = evtimer_new(ctx.base(), my_timer_cb, &ctx);
///   // ...
///
///   ctx.finish();
///   t.join();
/// @endcode
///
/// @note 不可复制/移动：event_base 持有内部状态，不支持移动语义。
class LibeventContext {
public:
	/// @brief 创建新的 event_base 实例。
	/// @throws std::runtime_error  event_base_new 返回 nullptr 时抛出。
	LibeventContext();

	/// @brief 释放 event_base。
	///
	/// @pre  必须在 run() 返回后调用（即已调用 finish() 且运行线程已 join）。
	~LibeventContext();

	LibeventContext(const LibeventContext&) = delete;
	LibeventContext& operator=(const LibeventContext&) = delete;

	// -------------------------------------------------------------------------
	// 调度器
	// -------------------------------------------------------------------------

	/// @brief 返回绑定到本上下文的 stdexec 调度器（PostingScheduler）。
	///
	/// 调度器将 schedule() 的执行映射到 post() → libevent 事件循环。
	detail::PostingScheduler<LibeventContext> scheduler() noexcept;

	// -------------------------------------------------------------------------
	// 生命周期
	// -------------------------------------------------------------------------

	/// @brief 在调用线程上运行 libevent 事件循环，阻塞直到 finish() 调用。
	///
	/// 内部调用 event_base_dispatch()；finish() 通过 event_base_loopbreak()
	/// 让循环在当前迭代结束后退出。
	void run();

	/// @brief 请求停止事件循环。
	///
	/// 线程安全，可从任意线程调用。内部通过 post() 投递 event_base_loopbreak()。
	void finish();

	// -------------------------------------------------------------------------
	// 任务投递
	// -------------------------------------------------------------------------

	/// @brief 将任务投递到 libevent 事件循环线程执行。
	///
	/// 线程安全：将 fn 加入内部队列后，通过 event_base_once(base, -1, EV_TIMEOUT,
	/// cb, this, &zero_timeval) 注册一个 0 超时的一次性事件，令回调在下一次
	/// event loop 迭代立即触发并批量执行队列中所有任务。
	///
	/// @param fn  要执行的任务；在 libevent 归属线程上调用。
	void post(std::function<void()> fn);

	// -------------------------------------------------------------------------
	// 底层访问
	// -------------------------------------------------------------------------

	/// @brief 返回底层 event_base 指针，供直接注册 libevent 事件使用。
	///
	/// @warning 在返回的 base 上注册的事件必须在 finish() 前删除，
	///          否则 event_base_dispatch 不会退出。
	event_base* base() noexcept;

private:
	/// @brief event_base_once 的回调；在归属线程上批量执行 pending_ 队列中的任务。
	static void on_dispatch(evutil_socket_t fd, short what, void* arg);

	/// libevent 事件循环实例。
	event_base* base_;

	/// 保护 pending_ 队列的互斥锁。
	std::mutex mutex_;

	/// 待执行任务队列；由 post() 写入，on_dispatch() 消费。
	std::queue<std::function<void()>> pending_;
};

} // namespace yi::coro
