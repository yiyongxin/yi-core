#pragma once

// 协程与普通线程之间的桥接工具
//
// 提供三类工具：
//   - spawn_on      从非协程线程启动协程，阻塞获取结果（future）
//   - submit        从非协程线程向执行上下文提交普通可调用对象，阻塞获取结果
//   - transfer_to   在协程内将执行迁移到另一上下文（非阻塞，co_await 友好）
//
// 所有工具均通过模板参数接受任意执行上下文，只要该上下文提供：
//   - ctx.scheduler()  返回满足 stdexec::scheduler 概念的调度器
//   - ctx.post(fn)     线程安全地将 fn 投递到归属线程执行
//
// C++26 迁移备注：
//   stdexec::* → std::execution::*，exec::start_detached → std::execution::spawn

#include "task.h"

#include <exec/start_detached.hpp>

#include <exception>
#include <future>
#include <memory>
#include <type_traits>

namespace yi::coro {

namespace detail {

// 包装协程：在目标 scheduler 上运行 task，完成后将结果写入 promise。
// GCC -Wsubobject-linkage：协程帧持有 stdexec 匿名命名空间类型，属已知诊断，已抑制。
STDEXEC_PRAGMA_PUSH()
STDEXEC_PRAGMA_IGNORE_GNU("-Wsubobject-linkage")
template <typename T>
yi::Task<void> spawn_wrapper(yi::Task<T> task, std::shared_ptr<std::promise<T>> prom) {
	try {
		if constexpr (std::is_void_v<T>) {
			co_await std::move(task);
			prom->set_value();
		} else {
			prom->set_value(co_await std::move(task));
		}
	} catch (...) {
		prom->set_exception(std::current_exception());
	}
}
STDEXEC_PRAGMA_POP()

} // namespace detail

/// @brief 从非协程线程向执行上下文提交协程，返回 future 供调用方阻塞等待。
///
/// @warning  禁止在协程内调用（会阻塞归属线程）；仅用于协程体外部的跨线程启动。
///
/// @tparam Ctx  执行上下文类型；须提供 scheduler() 返回 stdexec::scheduler。
/// @tparam T    协程返回值类型。
/// @param ctx   目标执行上下文；生命周期须覆盖 future::get() 调用。
/// @param task  要执行的协程。
/// @return      std::future<T>，调用 get() 阻塞直到协程完成或抛出异常。
template <typename Ctx, typename T>
std::future<T> spawn_on(Ctx& ctx, yi::Task<T> task) {
	auto prom = std::make_shared<std::promise<T>>();
	auto fut = prom->get_future();
	exec::start_detached(
	    stdexec::starts_on(ctx.scheduler(),
	                       detail::spawn_wrapper(std::move(task), std::move(prom))));
	return fut;
}

/// @brief 从非协程线程向执行上下文提交普通可调用对象，返回 future 供阻塞等待。
///
/// @warning  禁止在协程内调用（会阻塞归属线程）；仅用于协程体外部的跨线程提交。
///
/// @tparam Ctx  执行上下文类型；须提供 post(fn) 接口。
/// @tparam F    可调用类型；须满足 std::invocable。
/// @param ctx   目标执行上下文。
/// @param fn    要执行的可调用对象。
/// @return      std::future<invoke_result_t<F>>。
template <typename Ctx, std::invocable F>
auto submit(Ctx& ctx, F fn) -> std::future<std::invoke_result_t<F>> {
	using R = std::invoke_result_t<F>;
	auto prom = std::make_shared<std::promise<R>>();
	auto fut = prom->get_future();
	ctx.post([fn = std::move(fn), prom = std::move(prom)]() mutable {
		try {
			if constexpr (std::is_void_v<R>) {
				fn();
				prom->set_value();
			} else {
				prom->set_value(fn());
			}
		} catch (...) {
			prom->set_exception(std::current_exception());
		}
	});
	return fut;
}

/// @brief 在协程内将执行迁移到目标上下文并运行可调用对象，协程挂起直到完成。
///
/// co_await 后协程在 exec::task 的 sticky 调度器语义下自动回到原执行上下文，
/// 无需手动切换回来。
///
/// @tparam Ctx  执行上下文类型；须提供 scheduler()。
/// @tparam F    可调用类型；须满足 std::invocable。
/// @param ctx   目标执行上下文。
/// @param fn    要在 ctx 上执行的可调用对象；其返回值作为 co_await 的结果。
/// @return      stdexec sender，可在协程内 co_await。
template <typename Ctx, std::invocable F>
[[nodiscard]] auto transfer_to(Ctx& ctx, F fn) {
	return stdexec::then(
	    stdexec::starts_on(ctx.scheduler(), stdexec::just()),
	    std::move(fn));
}

/// @brief 在协程内将另一协程迁移到目标上下文执行，co_await 后回到原上下文。
///
/// @tparam Ctx  执行上下文类型；须提供 scheduler()。
/// @tparam T    被迁移协程的返回值类型。
/// @param ctx   目标执行上下文。
/// @param task  要在 ctx 上执行的协程。
/// @return      stdexec sender，可在协程内 co_await。
template <typename Ctx, typename T>
[[nodiscard]] auto transfer_to(Ctx& ctx, yi::Task<T> task) {
	return stdexec::starts_on(ctx.scheduler(), std::move(task));
}

} // namespace yi::coro
