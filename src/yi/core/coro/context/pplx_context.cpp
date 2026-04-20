#include <yi/core/coro/context/pplx_context.h>

namespace yi::coro {

detail::PostingScheduler<PplxContext> PplxContext::scheduler() noexcept {
	return {this};
}

void PplxContext::run() noexcept {}

void PplxContext::finish() noexcept {}

void PplxContext::post(std::function<void()> fn) {
	pplx::create_task([fn = std::move(fn)]() mutable { fn(); });
}

} // namespace yi::coro
