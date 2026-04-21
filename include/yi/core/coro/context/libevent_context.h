#pragma once

// libevent 执行上下文
// 头文件不引入任何 libevent 头文件；用户若需调用 libevent API 自行 #include <event2/event.h>。

#include <functional>
#include <mutex>
#include <queue>
#include "detail/posting_scheduler.h"

struct event_base; // 前向声明，避免引入 libevent 头文件

namespace yi::coro
{

// 基于 libevent event_base 的执行上下文，不可复制/移动。
class LibeventContext
{
public:
	LibeventContext();
	~LibeventContext();
	LibeventContext(const LibeventContext&)            = delete;
	LibeventContext& operator=(const LibeventContext&) = delete;
public:
	// 返回绑定到本上下文的 stdexec 调度器。
	detail::PostingScheduler<LibeventContext> scheduler() noexcept;
	// 在调用线程上运行事件循环，阻塞直到 finish()。
	void run();
	// 请求停止事件循环，线程安全。
	void finish();
	// 将任务投递到事件循环线程执行，线程安全。
	void post(std::function<void()> fn);
	// 返回底层 event_base*，finish() 前须先关闭已注册的事件。
	event_base* base() noexcept;
private:
	void drain_(); // 供内部 libevent 回调消费 pending_ 队列
private:
	event_base*                       base_;
	std::mutex                        mutex_;
	std::queue<std::function<void()>> pending_;
};

} // namespace yi::coro
