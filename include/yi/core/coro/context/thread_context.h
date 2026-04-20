#pragma once

// stdexec run_loop 驱动的单线程执行上下文
//
// C++26 迁移备注：
//   stdexec::run_loop → std::execution::run_loop

#include <exec/start_detached.hpp>
#include <stdexec/execution.hpp>

#include <concepts>
#include <functional>

namespace yi::coro {

/// @brief 基于 stdexec::run_loop 的单线程执行上下文。
///
/// 每个业务线程独占一个实例；所有协程/任务均在调用 run() 的归属线程上执行，
/// 保证严格的线程亲和性（exec::task 的 sticky 调度器语义依赖此特性）。
/// @note  不可复制/移动：run_loop 不可复制。
class ThreadContext {
public:
	ThreadContext() = default;
	~ThreadContext() = default;
	ThreadContext(const ThreadContext&) = delete;
	ThreadContext& operator=(const ThreadContext&) = delete;

	// -------------------------------------------------------------------------
	// 调度器
	// -------------------------------------------------------------------------

	/// @brief 返回绑定到本上下文 run_loop 的 stdexec 调度器。
	/// @note  必须内联（auto 返回类型推导由编译器在调用点完成）。
	stdexec::run_loop::scheduler scheduler() noexcept;

	// -------------------------------------------------------------------------
	// 生命周期
	// -------------------------------------------------------------------------

	/// @brief 在调用线程上启动事件循环，阻塞直到 finish() 被调用。
	/// @post  函数返回后，本上下文不再处理任何任务。
	void run();

	/// @brief 请求停止事件循环。
	/// 线程安全，可从任意线程调用。调用后 run() 将在当前迭代完成后返回。
	void finish();

	// -------------------------------------------------------------------------
	// 任务投递
	// -------------------------------------------------------------------------

	/// @brief 将可调用对象投递到本上下文的事件循环线程执行。
	///
	/// 线程安全，可从任意线程调用；任务按投递顺序在归属线程上顺序执行。
	/// @param fn  要执行的可调用对象，须满足 std::invocable。
	template <std::invocable F>
	void post(F fn);

private:
	/// 底层单线程事件循环，所有调度操作最终通过它序列化执行。
	stdexec::run_loop loop_;
};

// =============================================================================
// 内联/模板实现（必须在头文件中定义）
// =============================================================================

inline stdexec::run_loop::scheduler ThreadContext::scheduler() noexcept {
	return loop_.get_scheduler();
}

template <std::invocable F>
void ThreadContext::post(F fn) {
	exec::start_detached(
	    stdexec::then(stdexec::schedule(loop_.get_scheduler()), std::move(fn)));
}

} // namespace yi::coro
