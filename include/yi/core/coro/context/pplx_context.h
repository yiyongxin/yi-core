#pragma once

// pplx 线程池执行上下文
//
// 依赖：cpprestsdk（CMake: find_package(cpprestsdk) 或 pkg_check_modules(cpprest)）

#include "detail/posting_scheduler.h"

#include <pplx/pplxtasks.h>

#include <functional>

namespace yi::coro {

/// @brief 基于 pplx 线程池的执行上下文。
///
/// pplx 是 cpprestsdk 内置的跨平台任务调度器，维护自己的线程池。
/// 与 ThreadContext / LibuvContext 不同，PplxContext 无需独占线程，
/// 任务由 pplx 全局线程池调度执行。
///
/// 接口设计：
///   - scheduler() / post() 语义与其他上下文完全一致，可无缝用于
///     transfer_to、spawn_on 等工具。
///   - run() / finish() 为空操作（pplx 线程池生命周期由 cpprestsdk 全局管理）。
///
/// 典型使用场景：
///   作为 yi::http::from_pplx() 的 io_exec 参数，在 pplx 线程上执行
///   阻塞式 pplx_task.get()，从而不阻塞业务协程所在的执行上下文。
///
/// @code
///   yi::coro::PplxContext io_ctx;   // 可作全局/长生命周期对象
///   yi::coro::ThreadContext biz_ctx;
///   std::thread t([&]{ biz_ctx.run(); });
///
///   // 协程运行在 biz_ctx，IO 等待在 io_ctx（pplx 线程池）
///   auto fut = yi::coro::spawn_on(biz_ctx,
///       yi::http::from_pplx(io_ctx, some_pplx_task));
///
///   biz_ctx.finish();
///   t.join();
/// @endcode
///
/// 线程亲和性说明：
///   pplx 线程池不保证任务在固定线程上执行；多次 co_await 可能在不同
///   pplx 线程上恢复。若需线程亲和性，请使用 ThreadContext。
///
/// @note 可复制（无状态，仅作调度入口）；多个副本等价。
class PplxContext {
public:
	PplxContext() = default;
	~PplxContext() = default;

	// -------------------------------------------------------------------------
	// 调度器
	// -------------------------------------------------------------------------

	/// @brief 返回绑定到本上下文的 stdexec 调度器（PostingScheduler）。
	///
	/// 调度器将 schedule() 的执行映射到 post() → pplx 线程池。
	detail::PostingScheduler<PplxContext> scheduler() noexcept;

	// -------------------------------------------------------------------------
	// 生命周期（空操作）
	// -------------------------------------------------------------------------

	/// @brief 无操作。pplx 线程池由 cpprestsdk 全局管理，无需手动启动。
	void run() noexcept;

	/// @brief 无操作。pplx 线程池由 cpprestsdk 全局管理，无需手动停止。
	void finish() noexcept;

	// -------------------------------------------------------------------------
	// 任务投递
	// -------------------------------------------------------------------------

	/// @brief 将任务提交到 pplx 线程池执行（非阻塞，立即返回）。
	///
	/// 内部调用 pplx::create_task()，任务在 pplx 全局线程池的某个线程上执行。
	/// 线程安全：pplx::create_task 本身是线程安全的。
	///
	/// @param fn  要执行的任务。
	void post(std::function<void()> fn);
};

} // namespace yi::coro
