#pragma once

// C++26 迁移：stdexec::run_loop → std::execution::run_loop
#include <stdexec/execution.hpp>
#include <exec/start_detached.hpp>
#include <concepts>

namespace yi::coro
{

// 单线程事件循环。每个业务线程独占一个实例；协程仅在 run() 的归属线程上执行。
class ThreadExecutor
{
public:
    auto scheduler() noexcept
    {
        return loop_.get_scheduler();
    }

    void run()
    {
        loop_.run();
    }

    void finish()
    {
        loop_.finish();
    }

    // 线程安全：可从任意线程投递，在 run() 归属线程上顺序执行
    template <std::invocable F>
    void post(F fn)
    {
        exec::start_detached(
            stdexec::then(stdexec::schedule(loop_.get_scheduler()), std::move(fn))
        );
    }

private:
    stdexec::run_loop loop_;
};

} // namespace yi::coro
