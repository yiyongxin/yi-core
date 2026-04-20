#pragma once

// libevent execution context (brief)

#include <event2/event.h>
#include <functional>
#include <mutex>
#include <queue>

#include "detail/posting_scheduler.h"

namespace yi::coro 
{

// Libevent-based execution context. Provides `scheduler()`, `post()`, `run()`, `finish()`.
// Not copyable.
class LibeventContext 
{
public:
	// Create event_base.
	LibeventContext();
	// Destroy event_base (call after run() returned).
	~LibeventContext();
public:
	LibeventContext(const LibeventContext&) = delete;
	LibeventContext& operator=(const LibeventContext&) = delete;
	// Return a PostingScheduler bound to this context.
	detail::PostingScheduler<LibeventContext> scheduler() noexcept;
	// Run event loop on calling thread (blocks until finish()).
	void run();
	// Request loop stop (thread-safe).
	void finish();
	// 将任务投递到事件循环线程执行（线程安全）。
	void post(std::function<void()> fn);
	// 返回底层 event_base*（在 finish() 前请删除已注册的事件）。
	event_base* base() noexcept;
private:
	// Callback executing pending tasks.
	static void on_dispatch(evutil_socket_t fd, short what, void* arg);	
private:
	event_base* base_;
	std::mutex mutex_;
	std::queue<std::function<void()>> pending_;
};

} // namespace yi::coro
