#include <yi/core/coro/context/thread_context.h>

namespace yi::coro {

void ThreadContext::run() {
	loop_.run();
}

void ThreadContext::finish() {
	loop_.finish();
}

} // namespace yi::coro
