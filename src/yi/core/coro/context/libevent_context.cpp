#include <yi/core/coro/context/libevent_context.h>

#include <stdexcept>

namespace yi::coro {

LibeventContext::LibeventContext()
    : base_(event_base_new()) {
	if (!base_)
		throw std::runtime_error("event_base_new failed");
}

LibeventContext::~LibeventContext() {
	event_base_free(base_);
}

detail::PostingScheduler<LibeventContext> LibeventContext::scheduler() noexcept {
	return {this};
}

void LibeventContext::run() {
	event_base_dispatch(base_);
}

void LibeventContext::finish() {
	post([this] { event_base_loopbreak(base_); });
}

void LibeventContext::post(std::function<void()> fn) {
	{
		std::lock_guard<std::mutex> lk(mutex_);
		pending_.push(std::move(fn));
	}

	// event_base_once 注册一次性事件：fd=-1 表示纯超时，EV_TIMEOUT + {0,0}
	// 令回调在下一次 event loop 迭代立即执行。这是 libevent 跨线程 post 的标准手法。
	static constexpr timeval zero{0, 0};
	event_base_once(base_, -1, EV_TIMEOUT, &LibeventContext::on_dispatch, this, &zero);
}

event_base* LibeventContext::base() noexcept {
	return base_;
}

void LibeventContext::on_dispatch(evutil_socket_t /*fd*/, short /*what*/, void* arg) {
	auto* self = static_cast<LibeventContext*>(arg);

	std::queue<std::function<void()>> work;
	{
		std::lock_guard<std::mutex> lk(self->mutex_);
		std::swap(work, self->pending_);
	}
	while (!work.empty()) {
		work.front()();
		work.pop();
	}
}

} // namespace yi::coro
