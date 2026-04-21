#pragma once

// pplx 执行上下文（cpprestsdk 线程池）
// 类定义不依赖任何 pplx 头文件，实现细节在 .cpp 中。可拷贝。

#include <functional>
#include "detail/posting_scheduler.h"

namespace yi::coro
{

// 基于 pplx 全局线程池的执行上下文。run()/finish() 为无操作。
class PplxContext
{
public:
	PplxContext()  = default;
	~PplxContext() = default;
public:
	// 返回绑定到本上下文的 stdexec 调度器。
	detail::PostingScheduler<PplxContext> scheduler() noexcept;
	// 无操作，pplx 线程池生命周期由全局管理。
	void run() noexcept;
	// 无操作，pplx 线程池生命周期由全局管理。
	void finish() noexcept;
	// 将任务投递到 pplx 线程池执行，线程安全。
	void post(std::function<void()> fn);
};

} // namespace yi::coro
