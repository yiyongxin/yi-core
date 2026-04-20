#pragma once

// libuv execution context (brief)
// Depends on libuv (find_package(libuv)).

#include <functional>
#include <mutex>
#include <queue>

#include <uv.h>

#include "detail/posting_scheduler.h"

namespace yi::coro 
{
// Libuv-based execution context. Provides `scheduler()`, `post()`, `run()`, `finish()`.
// Not copyable.
class LibuvContext 
{
public:
	// Construct and initialize uv_loop and wakeup handle.
	LibuvContext();
	// Destroy loop (call after `finish()` and run() returned).
	~LibuvContext();
	LibuvContext(const LibuvContext&) = delete;
	LibuvContext& operator=(const LibuvContext&) = delete;
public:
	// Return a PostingScheduler bound to this context.
	detail::PostingScheduler<LibuvContext> scheduler() noexcept;
	// Run event loop on calling thread (blocks until finish()).
	void run();
	// Request loop stop (thread-safe).
	void finish();
	// Post a task to be executed on the loop's thread (thread-safe).
	void post(std::function<void()> fn);
	// Return underlying uv_loop_t* (registered handles must be closed before finish()).
	uv_loop_t* loop() noexcept;
private:
	// uv_async_t callback to execute pending tasks.
	static void on_wakeup(uv_async_t* handle);
private:
	// Event loop and wakeup handle.
	uv_loop_t loop_{};
	uv_async_t wakeup_{};
	// Pending task queue protected by mutex.
	std::mutex mutex_;
	std::queue<std::function<void()>> pending_;
};

} // namespace yi::coro
