#pragma once

// pplx execution context (brief)

#include "detail/posting_scheduler.h"
#include <pplx/pplxtasks.h>
#include <functional>

namespace yi::coro {

// Pplx-based execution context. Uses cpprestsdk's thread pool. Copyable.
class PplxContext {
public:
	PplxContext() = default;
	~PplxContext() = default;

	// Return a PostingScheduler bound to this context.
	detail::PostingScheduler<PplxContext> scheduler() noexcept;

	// No-op lifecycle: run/finish are managed by pplx.
	void run() noexcept;
	void finish() noexcept;

	// Post a task to the pplx thread pool.
	void post(std::function<void()> fn);
};

} // namespace yi::coro
