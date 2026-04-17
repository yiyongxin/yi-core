#pragma once

// yi::coro 跨线程桥接工具
//
// 原则：协程内部永远不处理线程同步。
// 跨线程的数据传递发生在协程的"外侧"，通过 std::promise/future 完成。
//
// 提供两个函数：
//   spawn_on(executor, task)  — 将协程投递到目标线程，返回 std::future<T>
//   submit(executor, fn)      — 将普通可调用对象投递到目标线程，返回 std::future<T>
//
// 调用方拿到 future 后：
//   - 可在协程外部调用 future.get() 阻塞等待结果
//   - 不应在协程体内部直接调用 future.get()（会阻塞事件循环）

#include "executor.h"
#include "task.h"

#include <exec/start_detached.hpp>
#include <stdexec/execution.hpp>

#include <exception>
#include <future>
#include <system_error>
#include <type_traits>
#include <utility>

namespace yi::coro {

namespace detail {

// 将 exec::task<T> 的完成信号桥接到 std::promise<T>
// 错误统一转为 std::exception_ptr；取消转为 operation_canceled 异常
template<typename T>
void start_with_promise(
    exec::task<T>   task,
    ThreadExecutor& target,
    std::shared_ptr<std::promise<T>> prom
) {
    auto chain = stdexec::on(target.scheduler(), std::move(task));

    if constexpr (std::is_void_v<T>) {
        exec::start_detached(
            std::move(chain)
            | stdexec::then([p = prom]() noexcept {
                p->set_value();
              })
            | stdexec::upon_error([p = prom](std::exception_ptr ep) noexcept {
                p->set_exception(std::move(ep));
              })
            | stdexec::upon_stopped([p = prom]() noexcept {
                p->set_exception(std::make_exception_ptr(
                    std::system_error(
                        std::make_error_code(std::errc::operation_canceled)
                    )
                ));
              })
        );
    } else {
        exec::start_detached(
            std::move(chain)
            | stdexec::then([p = prom](T val) noexcept {
                p->set_value(std::move(val));
              })
            | stdexec::upon_error([p = prom](std::exception_ptr ep) noexcept {
                p->set_exception(std::move(ep));
              })
            | stdexec::upon_stopped([p = prom]() noexcept {
                p->set_exception(std::make_exception_ptr(
                    std::system_error(
                        std::make_error_code(std::errc::operation_canceled)
                    )
                ));
              })
        );
    }
}

} // namespace detail

// ---------------------------------------------------------------------------
// spawn_on — 在 target 线程上启动协程，调用方获得 std::future<T>
//
// 用法（调用方线程，协程外部）：
//   std::future<int> fut = yi::coro::spawn_on(worker_exec, compute(42));
//   int result = fut.get();  // 阻塞直到协程完成
//
// 线程安全：可从任意线程调用
// ---------------------------------------------------------------------------
template<typename T>
std::future<T> spawn_on(ThreadExecutor& target, exec::task<T> task) {
    auto prom = std::make_shared<std::promise<T>>();
    std::future<T> fut = prom->get_future();
    detail::start_with_promise(std::move(task), target, std::move(prom));
    return fut;
}

// ---------------------------------------------------------------------------
// submit — 在 target 线程上执行普通可调用对象，调用方获得 std::future<T>
//
// 适用于非协程的 CPU 计算或同步 I/O，不应在 fn 内部再次进入事件循环。
//
// 用法（调用方线程，协程外部）：
//   std::future<int> fut = yi::coro::submit(worker_exec, []{ return heavy_compute(); });
//   int result = fut.get();
//
// 线程安全：可从任意线程调用
// ---------------------------------------------------------------------------
template<std::invocable F>
auto submit(ThreadExecutor& target, F fn)
    -> std::future<std::invoke_result_t<F>>
{
    using T = std::invoke_result_t<F>;
    auto prom = std::make_shared<std::promise<T>>();
    std::future<T> fut = prom->get_future();

    target.post([p = std::move(prom), f = std::move(fn)]() mutable {
        try {
            if constexpr (std::is_void_v<T>) {
                f();
                p->set_value();
            } else {
                p->set_value(f());
            }
        } catch (...) {
            p->set_exception(std::current_exception());
        }
    });

    return fut;
}

} // namespace yi::coro
