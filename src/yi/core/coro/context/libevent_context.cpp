#include <yi/core/coro/context/libevent_context.h>

#include <event2/event.h>

#include <stdexcept>

namespace yi::coro {

// 文件内部回调，签名由 libevent 要求，不出现在公共头文件中。
static void libevent_on_dispatch(evutil_socket_t /*fd*/, short /*what*/, void* arg) {
    static_cast<LibeventContext*>(arg)->drain_();
}

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
    static constexpr timeval zero{0, 0};
    event_base_once(base_, -1, EV_TIMEOUT, libevent_on_dispatch, this, &zero);
}

event_base* LibeventContext::base() noexcept {
    return base_;
}

void LibeventContext::drain_() {
    std::queue<std::function<void()>> work;
    {
        std::lock_guard<std::mutex> lk(mutex_);
        std::swap(work, pending_);
    }
    while (!work.empty()) {
        work.front()();
        work.pop();
    }
}

} // namespace yi::coro
