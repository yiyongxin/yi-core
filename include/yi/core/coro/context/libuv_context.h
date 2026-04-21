#pragma once

// libuv 执行上下文
// uv_loop_t/uv_async_t 为值成员，通过 PIMPL 隐藏在 .cpp 中，头文件无需引入 <uv.h>。
// uv_loop_t = struct uv_loop_s，前向声明供 loop() 返回类型使用。

#include <functional>
#include <memory>
#include "detail/posting_scheduler.h"

struct uv_loop_s; // 前向声明，避免引入 libuv 头文件

namespace yi::coro
{

// 基于 libuv uv_loop 的执行上下文，不可复制/移动。
class LibuvContext
{
public:
	LibuvContext();
	~LibuvContext();
	LibuvContext(const LibuvContext&)            = delete;
	LibuvContext& operator=(const LibuvContext&) = delete;
public:
	// 返回绑定到本上下文的 stdexec 调度器。
	detail::PostingScheduler<LibuvContext> scheduler() noexcept;
	// 在调用线程上运行事件循环，阻塞直到 finish()。
	void run();
	// 请求停止事件循环，线程安全。
	void finish();
	// 将任务投递到事件循环线程执行，线程安全。
	void post(std::function<void()> fn);
	// 返回底层 uv_loop_t*，finish() 前须先关闭已注册的句柄。
	uv_loop_s* loop() noexcept;
private:
	struct Impl;
	std::unique_ptr<Impl> impl_;
};

} // namespace yi::coro
