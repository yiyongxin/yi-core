# 协程使用规范

## 核心约束

> **协程只在发起的线程上运行。跨线程通信使用 promise/future 桥接，
> 不在协程函数内部处理线程同步。**

---

## 架构模型

```text
Thread A (业务线程)              Thread B (工作线程)
─────────────────────            ─────────────────────
ThreadExecutor exec_a            ThreadExecutor exec_b
exec_a.run() ← 驱动循环          exec_b.run() ← 驱动循环
│                                │
├─ Task<T> coro_1()              ├─ Task<T> coro_x()
│   co_await async_a()           │   co_await async_b()
│   co_await coro_2()            │
│                                │
│  std::future<R> ←── spawn_on ──┤
│  fut.get() 阻塞等待            │  (coro_x 在 Thread B 运行并完成)
```

---

## 快速开始

```cpp
#include <yi/core/coro/coro.h>
#include <thread>

yi::Task<int> compute(int x) {
    co_return x * 2;
}

int main() {
    yi::coro::ThreadExecutor exec;

    std::thread worker([&] { exec.run(); });

    // 投递协程，获取 future
    auto fut = yi::coro::spawn_on(exec, compute(21));
    int result = fut.get();  // 42

    exec.finish();
    worker.join();
}
```

---

## 正确模式

### 模式 1：单线程事件循环内的协程链

协程只 `co_await` 同一调度器上的任务，天然不跨线程。

```cpp
yi::Task<std::string> fetch_name(int id) {
    co_return "user_" + std::to_string(id);
}

yi::Task<std::string> build_greeting(int id) {
    // co_await 同线程上的另一协程 — 安全
    std::string name = co_await fetch_name(id);
    co_return "Hello, " + name;
}

// 调用方（协程外部，任意线程）：
auto fut = yi::coro::spawn_on(exec, build_greeting(1));
std::string msg = fut.get();
```

### 模式 2：跨线程获取结果（spawn_on）

跨线程的数据流动在协程**外部**通过 future 完成。

```cpp
// Thread A 启动 Thread B 上的协程，阻塞等待结果
yi::Task<Report> generate_report(Query q) { /* ... */ co_return report; }

// Thread A 的非协程代码：
std::future<Report> fut = yi::coro::spawn_on(worker_exec, generate_report(q));
// ... 做其他事 ...
Report r = fut.get();  // 阻塞直到 Thread B 完成
```

### 模式 3：跨线程提交普通函数（submit）

对于不需要协程的 CPU 密集型计算：

```cpp
std::future<int> fut = yi::coro::submit(worker_exec, [] {
    return heavy_compute();
});
int result = fut.get();
```

### 模式 4：多个线程互相投递

协程 A（Thread A）需要 Thread B 的结果时，
在**协程外的初始化阶段**提前提交任务，将 future 作为参数传入协程。

```cpp
// 非协程代码（任意线程）：
std::future<Data> data_fut = yi::coro::spawn_on(io_exec, load_data());

// 等待结果后再启动业务协程（或将 future 传入下一步）
Data data = data_fut.get();
auto process_fut = yi::coro::spawn_on(biz_exec, process(std::move(data)));
```

---

## 禁止模式

### 禁止 1：在协程体内调用 `future.get()`

```cpp
// ❌ 错误：阻塞事件循环线程，导致后续所有协程无法推进
yi::Task<void> bad() {
    auto fut = yi::coro::spawn_on(other_exec, some_task());
    auto result = fut.get();  // 死锁风险
    co_return;
}
```

### 禁止 2：在协程体内使用 `stdexec::transfer` 或 `stdexec::on(other_scheduler)`

```cpp
// ❌ 错误：协程在 Thread B 的调度器上继续运行，违反"不跨线程"约束
yi::Task<int> bad() {
    int val = co_await stdexec::transfer(
        stdexec::just(42),
        other_executor.scheduler()  // 迁移线程！
    );
    co_return val;
}
```

### 禁止 3：在协程体内创建 `std::thread` 或使用 `std::async`

```cpp
// ❌ 错误：绕过执行器模型，线程生命周期不可控
yi::Task<void> bad() {
    std::thread t([] { /* ... */ });
    t.detach();
    co_return;
}
```

### 禁止 4：在协程体内使用互斥锁

```cpp
// ❌ 错误：持有锁期间 co_await 可能导致锁永不释放
std::mutex mtx;

yi::Task<void> bad() {
    std::lock_guard<std::mutex> lk(mtx);  // 持锁
    co_await some_async_op();             // 协程挂起，锁继续持有
    co_return;
}
```

---

## CMake 集成

`yi_core` 已通过 `FetchContent` 自动引入 `STDEXEC::stdexec`。
下游项目链接 `yi::core` 即可获得所有协程支持，无需额外配置：

```cmake
target_link_libraries(my_app PRIVATE yi::core)
```

---

## API 速查

| 符号 | 头文件 | 说明 |
|---|---|---|
| `yi::Task<T>` | `coro/task.h` | 协程任务类型（`exec::task<T>` 别名） |
| `yi::coro::ThreadExecutor` | `coro/executor.h` | 单线程事件循环 |
| `yi::coro::spawn_on(exec, task)` | `coro/thread_bridge.h` | 跨线程投递协程 → `future<T>` |
| `yi::coro::submit(exec, fn)` | `coro/thread_bridge.h` | 跨线程投递函数 → `future<T>` |
| `#include <yi/core/coro/coro.h>` | — | 以上全部的统一入口 |
