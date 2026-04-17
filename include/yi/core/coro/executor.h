#pragma once

// yi::coro::ThreadExecutor — 单线程事件循环封装
//
// 每个业务线程持有一个 ThreadExecutor。
// 调用 run() 的线程即为该执行器的"归属线程"，所有协程在此线程上运行。
//
// 线程模型：
//   Thread A                    Thread B
//   ─────────────────────       ─────────────────────
//   ThreadExecutor exec_a;      ThreadExecutor exec_b;
//   exec_a.run();  ← 阻塞        exec_b.run();  ← 阻塞
//
//   协程只在 exec_a.run() 所在线程上运行，exec_b 同理。
//   跨线程通信见 thread_bridge.h。

#include <exec/start_detached.hpp>
#include <stdexec/execution.hpp>

namespace yi::coro {

class ThreadExecutor {
    stdexec::run_loop loop_;

public:
    using scheduler_type =
        decltype(std::declval<stdexec::run_loop&>().get_scheduler());

    // 返回绑定到本执行器的调度器，用于 stdexec::on(scheduler, task)
    [[nodiscard]] scheduler_type scheduler() noexcept {
        return loop_.get_scheduler();
    }

    // 阻塞调用线程，驱动事件循环直到 finish() 被调用
    // 必须由"归属线程"调用
    void run() { loop_.run(); }

    // 通知事件循环退出（线程安全，可从任意线程调用）
    void finish() { loop_.finish(); }

    // 投递一个可调用对象到本执行器的事件队列（线程安全）
    // fn 将在归属线程上被调用，不保证调用时机
    template<std::invocable F>
    void post(F fn) {
        exec::start_detached(
            stdexec::then(
                stdexec::schedule(loop_.get_scheduler()),
                std::move(fn)
            )
        );
    }

    ThreadExecutor() = default;
    ThreadExecutor(const ThreadExecutor&) = delete;
    ThreadExecutor& operator=(const ThreadExecutor&) = delete;
};

} // namespace yi::coro
