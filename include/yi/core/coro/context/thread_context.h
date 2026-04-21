#pragma once

// stdexec run_loop 驱动的单线程执行上下文
// C++26 迁移：stdexec::run_loop → std::execution::run_loop

#include <functional>
#include <stdexec/execution.hpp>

namespace yi::coro
{

// 基于 stdexec::run_loop 的单线程执行上下文，不可复制/移动。
// 所有协程/任务均在调用 run() 的归属线程上执行（exec::task sticky 调度器语义依赖此特性）。
class ThreadContext
{
public:
	ThreadContext()  = default;
	~ThreadContext() = default;
	ThreadContext(const ThreadContext&)            = delete;
	ThreadContext& operator=(const ThreadContext&) = delete;
public:
	// 返回绑定到本上下文 run_loop 的 stdexec 调度器。
	stdexec::run_loop::scheduler scheduler() noexcept;
	// 在调用线程上启动事件循环，阻塞直到 finish()。
	void run();
	// 请求停止事件循环，线程安全。
	void finish();
	// 将任务投递到本上下文的事件循环线程执行，线程安全。
	void post(std::function<void()> fn);
private:
	stdexec::run_loop loop_;
};

} // namespace yi::coro
