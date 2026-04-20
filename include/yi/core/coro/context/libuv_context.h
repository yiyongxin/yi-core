#pragma once

// libuv 事件循环执行上下文
//
// 依赖：libuv（CMake: find_package(libuv) 或 pkg_check_modules(LIBUV libuv)）

#include <functional>
#include <mutex>
#include <queue>

#include <uv.h>

#include "detail/posting_scheduler.h"

namespace yi::coro {

/// @brief 基于 libuv uv_loop_t 的执行上下文。
///
/// 将 libuv 事件循环封装为 yi::coro 标准执行上下文，提供与 ThreadContext
/// 完全一致的 scheduler() / post() / run() / finish() 接口。
///
/// 额外能力：
///   - loop() 暴露底层 uv_loop_t*，供业务代码直接注册 uv 句柄（如 tcp/timer/fs）。
///   - post() 与 libuv 原生句柄的回调天然共存于同一事件循环，无需额外同步。
///
/// 线程模型：
///   - run() 调用线程即"归属线程"，所有通过 post() 投递的任务在此线程执行。
///   - post() 本身线程安全，可从任意线程调用。
///
/// 生命周期注意事项：
///   - 析构前必须已调用 finish() 并等待 run() 返回（join 运行线程）；
///     否则 uv_loop_close 可能失败（存在未关闭句柄）。
///
/// 典型用法：
/// @code
///   LibuvContext ctx;
///   std::thread t([&]{ ctx.run(); });
///
///   // 注册一个 libuv 定时器
///   uv_timer_t timer;
///   uv_timer_init(ctx.loop(), &timer);
///   // ...
///
///   ctx.finish();
///   t.join();
/// @endcode
///
/// @note 不可复制/移动：uv 句柄内部持有 loop 指针，移动语义不安全。
class LibuvContext {
public:
	/// @brief 初始化 uv_loop_t 并注册跨线程唤醒用的 uv_async_t。
	/// @throws std::runtime_error  uv_loop_init 或 uv_async_init 失败时抛出。
	LibuvContext();

	/// @brief 关闭并释放 uv_loop_t。
	///
	/// @pre  必须在 run() 返回后调用（即已调用 finish() 且运行线程已 join）。
	~LibuvContext();

	LibuvContext(const LibuvContext&) = delete;
	LibuvContext& operator=(const LibuvContext&) = delete;

	// -------------------------------------------------------------------------
	// 调度器
	// -------------------------------------------------------------------------

	/// @brief 返回绑定到本上下文的 stdexec 调度器（PostingScheduler）。
	///
	/// 调度器将 schedule() 的执行映射到 post() → libuv 事件循环。
	detail::PostingScheduler<LibuvContext> scheduler() noexcept;

	// -------------------------------------------------------------------------
	// 生命周期
	// -------------------------------------------------------------------------

	/// @brief 在调用线程上运行 libuv 事件循环（UV_RUN_DEFAULT），阻塞直到
	///        finish() 关闭所有活跃句柄后循环自然退出。
	void run();

	/// @brief 请求停止事件循环。
	///
	/// 内部通过 post() 投递一个关闭 wakeup 句柄的任务；句柄关闭后 libuv
	/// 无活跃句柄，run() 自然返回。线程安全，可从任意线程调用。
	void finish();

	// -------------------------------------------------------------------------
	// 任务投递
	// -------------------------------------------------------------------------

	/// @brief 将任务投递到 libuv 事件循环线程执行。
	///
	/// 线程安全：将 fn 加入内部队列后调用 uv_async_send 唤醒事件循环；
	/// 多次并发调用会被 libuv 合并为一次唤醒（coalescing），队列中所有
	/// 待执行任务在同一次 wakeup 回调中批量执行。
	///
	/// @param fn  要执行的任务；在 libuv 归属线程上调用。
	void post(std::function<void()> fn);

	// -------------------------------------------------------------------------
	// 底层访问
	// -------------------------------------------------------------------------

	/// @brief 返回底层 uv_loop_t 指针，供直接注册 libuv 句柄使用。
	///
	/// @warning 在返回的 loop 上注册的句柄必须在 finish() 前关闭，
	///          否则 run() 不会返回（libuv 循环存在活跃句柄）。
	uv_loop_t* loop() noexcept;

private:
	/// @brief uv_async_t 的回调；在归属线程上批量执行 pending_ 队列中的所有任务。
	static void on_wakeup(uv_async_t* handle);

	/// libuv 事件循环实例，驱动所有 uv 句柄和定时器。
	uv_loop_t loop_{};

	/// 跨线程唤醒句柄；post() 通过 uv_async_send 触发，保证线程安全。
	uv_async_t wakeup_{};

	/// 保护 pending_ 队列的互斥锁。
	std::mutex mutex_;

	/// 待执行任务队列；由 post() 写入，on_wakeup() 消费。
	std::queue<std::function<void()>> pending_;
};

} // namespace yi::coro
