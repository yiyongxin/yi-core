#include <yi/core/coro/context/libuv_context.h>

#include <stdexcept>

namespace yi::coro {

LibuvContext::LibuvContext() {
	if (uv_loop_init(&loop_) != 0)
		throw std::runtime_error("uv_loop_init failed");

	if (uv_async_init(&loop_, &wakeup_, &LibuvContext::on_wakeup) != 0) {
		uv_loop_close(&loop_);
		throw std::runtime_error("uv_async_init failed");
	}
	wakeup_.data = this;
}

LibuvContext::~LibuvContext() {
	// 调用方须保证 run() 已返回（finish() + join）；
	// 此时 loop 中无活跃句柄，uv_loop_close 可安全调用。
	uv_loop_close(&loop_);
}

detail::PostingScheduler<LibuvContext> LibuvContext::scheduler() noexcept {
	return {this};
}

void LibuvContext::run() {
	uv_run(&loop_, UV_RUN_DEFAULT);
}

void LibuvContext::finish() {
	// 投递一个关闭 wakeup 句柄的任务；句柄关闭后 loop 无活跃句柄，run() 自然退出
	post([this] {
		uv_close(reinterpret_cast<uv_handle_t*>(&wakeup_), nullptr);
	});
}

void LibuvContext::post(std::function<void()> fn) {
	{
		std::lock_guard<std::mutex> lk(mutex_);
		pending_.push(std::move(fn));
	}
	uv_async_send(&wakeup_);
}

uv_loop_t* LibuvContext::loop() noexcept {
	return &loop_;
}

void LibuvContext::on_wakeup(uv_async_t* handle) {
	auto* self = static_cast<LibuvContext*>(handle->data);

	// 将队列整体 swap 出来，避免在锁外执行任务期间持有锁
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
