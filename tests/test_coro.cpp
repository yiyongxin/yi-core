// 协程模块单元测试
//
// CoroTask
//   VoidReturn                  — Task<void> 正常完成
//   ValueReturn                 — Task<int> 返回标量值
//   StringReturn                — Task<string> 返回字符串
//   ChainedAwait                — 协程 co_await 另一协程，结果正确传递
//   DeepChain                   — 10 级嵌套 co_await，结果从最深层正确传递
//   MultipleCoawait             — 同一协程内多次顺序 co_await，累计结果正确
//   ExceptionPropagates         — 协程内抛异常，spawn_on 的 future 携带该异常
//   NestedExceptionPropagates   — 深层嵌套协程抛异常，逐层传播到 future
//
// CoroExecutor
//   PostRunsOnOwnerThread   — post() 投递的任务在 run() 线程上执行
//   FinishStopsLoop         — finish() 后 run() 返回
//   MultiplePostOrdered     — 多次 post() 按顺序在归属线程执行
//   PostBeforeRun           — run() 前调用 post()，任务在 run() 启动后正常执行
//   PostFromOwnerThread     — 在归属线程回调内再次 post()，任务正常执行
//
// CoroThreadBridge
//   SpawnOnVoid             — spawn_on Task<void>，future 正常就绪
//   SpawnOnInt              — spawn_on Task<int>，future 携带正确值
//   SpawnOnException        — spawn_on 协程抛异常，future.get() 重新抛出
//   SubmitCallable          — submit 普通函数，future 携带返回值
//   SubmitVoid              — submit void 函数，future 正常就绪
//   SubmitException         — submit 抛异常，future.get() 重新抛出
//   SubmitRunsOnTargetThread — submit callable 在目标 executor 线程上执行
//   ConcurrentSpawn         — 并发 spawn_on 多个协程，所有结果正确
//   ConcurrentSubmit        — 多线程并发 submit，所有结果正确
//   SpawnOnTwoExecutors     — 向两个独立 executor 同时 spawn_on，结果互不干扰

#include <yi/core/coro/coro.h>

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <stdexec/execution.hpp>
#include <string>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

// ---------------------------------------------------------------------------
// 辅助：在独立线程上运行 ThreadExecutor，RAII 析构时 finish + join
// ---------------------------------------------------------------------------
struct ExecutorThread {
    yi::coro::ThreadExecutor exec;
    std::thread              thread;

    ExecutorThread()
        : thread([this] { exec.run(); })
    {}

    ~ExecutorThread() {
        exec.finish();
        thread.join();
    }
};

// ---------------------------------------------------------------------------
// 测试协程（仅在 ThreadExecutor 归属线程上运行）
// ---------------------------------------------------------------------------

yi::Task<void> coro_void() {
    co_return;
}

yi::Task<int> coro_return_int(int val) {
    co_return val;
}

yi::Task<std::string> coro_return_string(std::string s) {
    co_return s;
}

yi::Task<int> coro_chain(int x) {
    int doubled = co_await coro_return_int(x * 2);
    int quadrupled = co_await coro_return_int(doubled * 2);
    co_return quadrupled;
}

yi::Task<int> coro_throws() {
    throw std::runtime_error("coro error");
    co_return 0;
}

yi::Task<int> coro_deep(int depth, int val) {
    if (depth == 0) co_return val;
    co_return co_await coro_deep(depth - 1, val + 1);
}

yi::Task<int> coro_multi_await() {
    int a = co_await coro_return_int(1);
    int b = co_await coro_return_int(2);
    int c = co_await coro_return_int(3);
    co_return a + b + c;
}

yi::Task<int> coro_inner_throw() {
    throw std::runtime_error("inner error");
    co_return 0;
}

yi::Task<int> coro_middle() {
    co_return co_await coro_inner_throw();
}

yi::Task<int> coro_outer() {
    co_return co_await coro_middle();
}

// ---------------------------------------------------------------------------
// CoroTask
// ---------------------------------------------------------------------------

TEST(CoroTask, VoidReturn) {
    ExecutorThread et;
    auto fut = yi::coro::spawn_on(et.exec, coro_void());
    ASSERT_NO_THROW(fut.get());
}

TEST(CoroTask, ValueReturn) {
    ExecutorThread et;
    auto fut = yi::coro::spawn_on(et.exec, coro_return_int(42));
    EXPECT_EQ(fut.get(), 42);
}

TEST(CoroTask, StringReturn) {
    ExecutorThread et;
    auto fut = yi::coro::spawn_on(et.exec, coro_return_string("hello"));
    EXPECT_EQ(fut.get(), "hello");
}

TEST(CoroTask, ChainedAwait) {
    ExecutorThread et;
    // coro_chain(3) → 3*2=6 → 6*2=12
    auto fut = yi::coro::spawn_on(et.exec, coro_chain(3));
    EXPECT_EQ(fut.get(), 12);
}

TEST(CoroTask, ExceptionPropagates) {
    ExecutorThread et;
    auto fut = yi::coro::spawn_on(et.exec, coro_throws());
    EXPECT_THROW(fut.get(), std::runtime_error);
}

// ---------------------------------------------------------------------------
// CoroExecutor
// ---------------------------------------------------------------------------

TEST(CoroExecutor, PostRunsOnOwnerThread) {
    yi::coro::ThreadExecutor exec;
    std::thread::id          owner_id;
    std::thread::id          post_id;
    std::promise<void>       done;
    auto                     done_fut = done.get_future();

    std::thread worker([&] {
        owner_id = std::this_thread::get_id();
        exec.post([&] {
            post_id = std::this_thread::get_id();
            done.set_value();
            exec.finish();
        });
        exec.run();
    });

    done_fut.get();
    worker.join();

    EXPECT_EQ(post_id, owner_id);
    EXPECT_NE(post_id, std::this_thread::get_id());
}

TEST(CoroExecutor, FinishStopsLoop) {
    yi::coro::ThreadExecutor exec;
    std::atomic<bool>        loop_exited{false};

    std::thread worker([&] {
        exec.run();
        loop_exited.store(true, std::memory_order_relaxed);
    });

    exec.finish();
    worker.join();

    EXPECT_TRUE(loop_exited.load());
}

TEST(CoroExecutor, MultiplePostOrdered) {
    yi::coro::ThreadExecutor exec;
    std::vector<int>         order;
    std::mutex               mtx;
    std::promise<void>       done;
    auto                     done_fut = done.get_future();

    std::thread worker([&] { exec.run(); });

    for (int i = 0; i < 5; ++i) {
        exec.post([i, &order, &mtx] {
            std::lock_guard<std::mutex> lk(mtx);
            order.push_back(i);
        });
    }
    exec.post([&done, &exec] {
        done.set_value();
        exec.finish();
    });

    done_fut.get();
    worker.join();

    EXPECT_EQ(order, (std::vector<int>{0, 1, 2, 3, 4}));
}

// ---------------------------------------------------------------------------
// CoroThreadBridge
// ---------------------------------------------------------------------------

TEST(CoroThreadBridge, SpawnOnVoid) {
    ExecutorThread et;
    auto           fut = yi::coro::spawn_on(et.exec, coro_void());
    ASSERT_NO_THROW(fut.get());
}

TEST(CoroThreadBridge, SpawnOnInt) {
    ExecutorThread et;
    auto           fut = yi::coro::spawn_on(et.exec, coro_return_int(99));
    EXPECT_EQ(fut.get(), 99);
}

TEST(CoroThreadBridge, SpawnOnException) {
    ExecutorThread et;
    auto           fut = yi::coro::spawn_on(et.exec, coro_throws());
    try {
        fut.get();
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        EXPECT_STREQ(e.what(), "coro error");
    }
}

TEST(CoroThreadBridge, SubmitCallable) {
    ExecutorThread et;
    auto           fut = yi::coro::submit(et.exec, [] { return 7 * 6; });
    EXPECT_EQ(fut.get(), 42);
}

TEST(CoroThreadBridge, SubmitVoid) {
    ExecutorThread et;
    std::atomic<bool> called{false};
    auto fut = yi::coro::submit(et.exec, [&called] {
        called.store(true, std::memory_order_relaxed);
    });
    ASSERT_NO_THROW(fut.get());
    EXPECT_TRUE(called.load());
}

TEST(CoroThreadBridge, SubmitException) {
    ExecutorThread et;
    auto           fut = yi::coro::submit(et.exec, []() -> int {
        throw std::logic_error("submit error");
    });
    EXPECT_THROW(fut.get(), std::logic_error);
}

TEST(CoroThreadBridge, ConcurrentSpawn) {
    ExecutorThread et;

    constexpr int              N = 8;
    std::vector<std::future<int>> futures;
    futures.reserve(N);

    for (int i = 0; i < N; ++i) {
        futures.push_back(yi::coro::spawn_on(et.exec, coro_return_int(i * i)));
    }

    for (int i = 0; i < N; ++i) {
        EXPECT_EQ(futures[i].get(), i * i) << "i=" << i;
    }
}

// ---------------------------------------------------------------------------
// CoroTask（补充）
// ---------------------------------------------------------------------------

TEST(CoroTask, DeepChain) {
    ExecutorThread et;
    // coro_deep(10, 0): 递归 10 次，每次 val+1，结果为 10
    auto fut = yi::coro::spawn_on(et.exec, coro_deep(10, 0));
    EXPECT_EQ(fut.get(), 10);
}

TEST(CoroTask, MultipleCoawait) {
    ExecutorThread et;
    // coro_multi_await: 1 + 2 + 3 = 6
    auto fut = yi::coro::spawn_on(et.exec, coro_multi_await());
    EXPECT_EQ(fut.get(), 6);
}

TEST(CoroTask, NestedExceptionPropagates) {
    ExecutorThread et;
    // coro_outer → coro_middle → coro_inner_throw，异常逐层传播
    auto fut = yi::coro::spawn_on(et.exec, coro_outer());
    try {
        fut.get();
        FAIL() << "Expected std::runtime_error";
    } catch (const std::runtime_error& e) {
        EXPECT_STREQ(e.what(), "inner error");
    }
}

// ---------------------------------------------------------------------------
// CoroExecutor（补充）
// ---------------------------------------------------------------------------

TEST(CoroExecutor, PostBeforeRun) {
    yi::coro::ThreadExecutor exec;
    std::atomic<int>   counter{0};
    std::promise<void> done;
    auto               done_fut = done.get_future();

    for (int i = 0; i < 3; ++i) {
        exec.post([&counter] {
            counter.fetch_add(1, std::memory_order_relaxed);
        });
    }
    exec.post([&done, &exec] {
        done.set_value();
        exec.finish();
    });

    std::thread worker([&] { exec.run(); });
    done_fut.get();
    worker.join();

    EXPECT_EQ(counter.load(), 3);
}

TEST(CoroExecutor, PostFromOwnerThread) {
    yi::coro::ThreadExecutor exec;
    std::atomic<int>   counter{0};
    std::promise<void> done;
    auto               done_fut = done.get_future();

    std::thread worker([&] { exec.run(); });

    exec.post([&exec, &counter, &done] {
        counter.fetch_add(1, std::memory_order_relaxed);
        exec.post([&exec, &counter, &done] {
            counter.fetch_add(1, std::memory_order_relaxed);
            done.set_value();
            exec.finish();
        });
    });

    done_fut.get();
    worker.join();

    EXPECT_EQ(counter.load(), 2);
}

// ---------------------------------------------------------------------------
// CoroThreadBridge（补充）
// ---------------------------------------------------------------------------

TEST(CoroThreadBridge, SubmitRunsOnTargetThread) {
    ExecutorThread et;

    auto fut1 = yi::coro::submit(et.exec, [] { return std::this_thread::get_id(); });
    auto fut2 = yi::coro::submit(et.exec, [] { return std::this_thread::get_id(); });

    std::thread::id id1 = fut1.get();
    std::thread::id id2 = fut2.get();

    EXPECT_EQ(id1, id2);
    EXPECT_NE(id1, std::this_thread::get_id());
}

TEST(CoroThreadBridge, ConcurrentSubmit) {
    ExecutorThread et;
    constexpr int N = 16;

    std::vector<std::future<int>> futures;
    std::mutex                    fut_mtx;

    std::vector<std::thread> senders;
    senders.reserve(N);
    for (int i = 0; i < N; ++i) {
        senders.emplace_back([i, &et, &futures, &fut_mtx] {
            auto fut = yi::coro::submit(et.exec, [i] { return i * 3; });
            std::lock_guard<std::mutex> lk(fut_mtx);
            futures.push_back(std::move(fut));
        });
    }
    for (auto& t : senders) t.join();

    std::vector<int> results;
    results.reserve(N);
    for (auto& f : futures) results.push_back(f.get());

    std::vector<int> expected;
    for (int i = 0; i < N; ++i) expected.push_back(i * 3);

    std::sort(results.begin(), results.end());
    std::sort(expected.begin(), expected.end());
    EXPECT_EQ(results, expected);
}

TEST(CoroThreadBridge, SpawnOnTwoExecutors) {
    ExecutorThread et1;
    ExecutorThread et2;

    auto fut1 = yi::coro::spawn_on(et1.exec, coro_return_int(10));
    auto fut2 = yi::coro::spawn_on(et2.exec, coro_return_int(20));

    EXPECT_EQ(fut1.get(), 10);
    EXPECT_EQ(fut2.get(), 20);
}
