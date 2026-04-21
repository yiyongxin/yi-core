#include <yi/core/coro/context/libuv_context.h>

#include <uv.h>

#include <functional>
#include <mutex>
#include <queue>
#include <stdexcept>

namespace yi::coro {

struct LibuvContext::Impl {
    uv_loop_t  loop{};
    uv_async_t wakeup{};
    std::mutex mutex;
    std::queue<std::function<void()>> pending;

    static void on_wakeup(uv_async_t* handle) {
        auto* impl = static_cast<Impl*>(handle->data);

        std::queue<std::function<void()>> work;
        {
            std::lock_guard<std::mutex> lk(impl->mutex);
            std::swap(work, impl->pending);
        }
        while (!work.empty()) {
            work.front()();
            work.pop();
        }
    }
};

LibuvContext::LibuvContext()
    : impl_(std::make_unique<Impl>()) {
    if (uv_loop_init(&impl_->loop) != 0)
        throw std::runtime_error("uv_loop_init failed");

    if (uv_async_init(&impl_->loop, &impl_->wakeup, Impl::on_wakeup) != 0) {
        uv_loop_close(&impl_->loop);
        throw std::runtime_error("uv_async_init failed");
    }
    impl_->wakeup.data = impl_.get();
}

LibuvContext::~LibuvContext() {
    uv_loop_close(&impl_->loop);
}

detail::PostingScheduler<LibuvContext> LibuvContext::scheduler() noexcept {
    return {this};
}

void LibuvContext::run() {
    uv_run(&impl_->loop, UV_RUN_DEFAULT);
}

void LibuvContext::finish() {
    post([this] {
        uv_close(reinterpret_cast<uv_handle_t*>(&impl_->wakeup), nullptr);
    });
}

void LibuvContext::post(std::function<void()> fn) {
    {
        std::lock_guard<std::mutex> lk(impl_->mutex);
        impl_->pending.push(std::move(fn));
    }
    uv_async_send(&impl_->wakeup);
}

uv_loop_s* LibuvContext::loop() noexcept {
    return &impl_->loop;
}

} // namespace yi::coro
