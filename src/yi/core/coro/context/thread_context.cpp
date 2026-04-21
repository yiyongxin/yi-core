#include <yi/core/coro/context/thread_context.h>

#include <exec/start_detached.hpp>

namespace yi::coro {

stdexec::run_loop::scheduler ThreadContext::scheduler() noexcept {
    return loop_.get_scheduler();
}

void ThreadContext::run() {
    loop_.run();
}

void ThreadContext::finish() {
    loop_.finish();
}

void ThreadContext::post(std::function<void()> fn) {
    exec::start_detached(
        stdexec::then(stdexec::schedule(loop_.get_scheduler()), std::move(fn)));
}

} // namespace yi::coro
