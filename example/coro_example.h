#pragma once

#include "yi/core/coro/coro.h"  // IWYU pragma: keep

#include <cassert>
#include <string>
#include <thread>

using namespace std;
using namespace yi;
// ─── 协程模块使用示例 ──────────────────────────────────────────────────────────
//
// 核心约定：
//   · 协程（Task<T>）只在归属 executor 的线程上运行
//   · 跨线程需求分两类：
//       外部线程发起  → spawn_on / submit，caller 用 future.get() 阻塞等待
//       协程内部发起  → transfer_to，co_await 后自动回到原线程，不阻塞任何线程
//   · transfer_to 的线程安全保证来自 exec::task 的 sticky 调度器亲和性
//
// 注：示例协程函数含 stdexec 内部匿名命名空间类型，GCC 会产生 -Wsubobject-linkage
//     警告，属已知编译器诊断，不影响正确性。生产代码应将协程定义置于 .cpp 文件。

// ─── 示例用协程 ───────────────────────────────────────────────────────────────

// 模拟耗时计算（实际场景为 IO / CPU 密集型操作）
STDEXEC_PRAGMA_PUSH()
STDEXEC_PRAGMA_IGNORE_GNU("-Wsubobject-linkage")

static Task<int> coro_compute(int x)
{
    co_return x * 2;
}

// ─── 示例1：外部线程启动协程（spawn_on） ─────────────────────────────────────
//
// 适用场景：从普通线程（非协程）发起协程任务，阻塞等待结果。
// future.get() 阻塞的是 caller 线程，executor 线程不受影响。

inline void example_spawn_from_thread()
{
    coro::ThreadExecutor exec;
    std::thread              t([&] { exec.run(); });

    auto fut = coro::spawn_on(exec, coro_compute(21));
    int  r   = fut.get();
    assert(r == 42);

    exec.finish();
    t.join();
}

// ─── 示例2：协程内跨线程调用 callable（transfer_to + lambda） ────────────────
//
// 适用场景：协程内需将 CPU/IO 密集操作 offload 到 worker 线程。
// co_await 挂起协程（不阻塞线程）；完成后 sticky 亲和性自动回到原线程。

static Task<std::string> example_transfer_callable(coro::ThreadExecutor& worker)
{
    auto origin = std::this_thread::get_id();

    std::string result = co_await coro::transfer_to(worker, [] {
        return std::string("computed on worker");
    });

    assert(std::this_thread::get_id() == origin);  // co_await 前后同一线程
    co_return result;
}

// ─── 示例3：协程内跨线程调用另一协程（transfer_to + Task） ───────────────────
//
// 适用场景：子协程需在指定线程上运行（如 IO 线程、渲染线程）。
// 子协程整体在 worker 线程执行；外层协程 co_await 后回到自己的线程。

static Task<int> example_transfer_task(coro::ThreadExecutor& worker)
{
    auto origin = std::this_thread::get_id();

    int r = co_await coro::transfer_to(worker, coro_compute(21));

    assert(std::this_thread::get_id() == origin);  // co_await 前后同一线程
    co_return r;
}

// ─── 示例4：多次跨线程调用，每次 co_await 后都回到原线程 ─────────────────────
//
// 适用场景：协程内需多次向 worker 提交任务并逐步汇总结果。
// 数据（a、b）始终在同一线程访问，无跨线程竞争，无需加锁。

static Task<int> example_multi_hop(coro::ThreadExecutor& worker)
{
    auto origin = std::this_thread::get_id();

    int a = co_await coro::transfer_to(worker, [] { return 10; });
    assert(std::this_thread::get_id() == origin);   // 第一次回来

    int b = co_await coro::transfer_to(worker, [] { return 32; });
    assert(std::this_thread::get_id() == origin);   // 第二次回来

    co_return a + b;   // 42，a / b 的读写全在同一线程
}

// ─── 示例5：两个 executor 协同 ────────────────────────────────────────────────
//
// 适用场景：多级 worker 流水线，每级结果自动带回原线程再转发到下一级。

static Task<int> example_two_executors(
    coro::ThreadExecutor& worker_a,
    coro::ThreadExecutor& worker_b)
{
    int step1 = co_await coro::transfer_to(worker_a, [] { return 6; });
    int step2 = co_await coro::transfer_to(worker_b, [step1] { return step1 * 7; });
    co_return step2;   // 42
}

STDEXEC_PRAGMA_POP()

// ─── 驱动示例（普通线程调用入口） ────────────────────────────────────────────

inline void example_run_multi_hop()
{
    coro::ThreadExecutor main_exec;
    coro::ThreadExecutor worker_exec;

    std::thread main_t([&]   { main_exec.run(); });
    std::thread worker_t([&] { worker_exec.run(); });

    auto fut = coro::spawn_on(main_exec, example_multi_hop(worker_exec));
    int  r   = fut.get();
    assert(r == 42);

    worker_exec.finish();
    worker_t.join();
    main_exec.finish();
    main_t.join();
}

inline void example_run_two_executors()
{
    coro::ThreadExecutor main_exec;
    coro::ThreadExecutor worker_a;
    coro::ThreadExecutor worker_b;

    std::thread main_t([&] { main_exec.run(); });
    std::thread t_a([&]    { worker_a.run(); });
    std::thread t_b([&]    { worker_b.run(); });

    auto fut = coro::spawn_on(main_exec, example_two_executors(worker_a, worker_b));
    int  r   = fut.get();
    assert(r == 42);

    worker_b.finish(); t_b.join();
    worker_a.finish(); t_a.join();
    main_exec.finish(); main_t.join();
}
