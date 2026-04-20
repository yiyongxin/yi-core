#pragma once

// C++26 迁移：stdexec::* → std::execution::*，exec::start_detached → std::execution::spawn
#include "executor.h"
#include "task.h"

#include <exec/start_detached.hpp>

#include <exception>
#include <future>
#include <memory>
#include <type_traits>

namespace yi::coro
{

namespace detail
{

// 包装协程：在目标 executor 上运行 task，完成后将结果写入 promise
// GCC -Wsubobject-linkage：协程帧持有 stdexec 匿名命名空间类型，属已知诊断
STDEXEC_PRAGMA_PUSH()
STDEXEC_PRAGMA_IGNORE_GNU("-Wsubobject-linkage")
template <typename T>
yi::Task<void> spawn_wrapper(yi::Task<T> task, std::shared_ptr<std::promise<T>> prom)
{
    try
    {
        if constexpr (std::is_void_v<T>)
        {
            co_await std::move(task);
            prom->set_value();
        }
        else
        {
            prom->set_value(co_await std::move(task));
        }
    }
    catch (...)
    {
        prom->set_exception(std::current_exception());
    }
}
STDEXEC_PRAGMA_POP()

} // namespace detail

// 协程外部跨线程提交：caller 阻塞在 future.get()，禁止在协程内调用
template <typename T>
std::future<T> spawn_on(ThreadExecutor& exec, yi::Task<T> task)
{
    auto prom = std::make_shared<std::promise<T>>();
    auto fut  = prom->get_future();
    exec::start_detached(
        stdexec::starts_on(exec.scheduler(), detail::spawn_wrapper(std::move(task), std::move(prom)))
    );
    return fut;
}

// 协程外部跨线程提交：caller 阻塞在 future.get()，禁止在协程内调用
template <std::invocable F>
auto submit(ThreadExecutor& exec, F fn) -> std::future<std::invoke_result_t<F>>
{
    using R = std::invoke_result_t<F>;
    auto prom = std::make_shared<std::promise<R>>();
    auto fut  = prom->get_future();
    exec.post([fn = std::move(fn), prom = std::move(prom)]() mutable {
        try
        {
            if constexpr (std::is_void_v<R>)
            {
                fn();
                prom->set_value();
            }
            else
            {
                prom->set_value(fn());
            }
        }
        catch (...)
        {
            prom->set_exception(std::current_exception());
        }
    });
    return fut;
}

// 在协程内跨线程运行 callable，exec::task sticky 亲和性保证 co_await 后在原线程恢复
template <std::invocable F>
[[nodiscard]] auto transfer_to(ThreadExecutor& target, F fn)
{
    return stdexec::then(
        stdexec::starts_on(target.scheduler(), stdexec::just()),
        std::move(fn)
    );
}

// 在协程内跨线程运行另一协程，exec::task sticky 亲和性保证 co_await 后在原线程恢复
template <typename T>
[[nodiscard]] auto transfer_to(ThreadExecutor& target, yi::Task<T> task)
{
    return stdexec::starts_on(target.scheduler(), std::move(task));
}

} // namespace yi::coro
