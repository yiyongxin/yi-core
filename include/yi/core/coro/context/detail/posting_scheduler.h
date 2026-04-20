#pragma once

// 通用 posting 调度器（内部实现细节）
//
// 将任何实现了 `post(std::function<void()>)` 的执行上下文
// 适配为满足 stdexec::scheduler 概念的调度器对象。
//
// 设计：
//   - 采用 tag_invoke 扩展点，兼容 P2300 / stdexec 当前主线版本。
//   - 模板参数 Context 须满足：void Context::post(std::function<void()>)
//   - 该头文件仅供 yi::coro::*Context 内部使用，外部代码无需直接包含。

#include <stdexec/execution.hpp>

#include <exception>
#include <functional>

namespace yi::coro::detail {

/// @brief 基于 post() 的通用 stdexec 调度器。
///
/// 每个 *Context 类通过持有指向自身的指针构造此对象，
/// 从而将 stdexec::schedule() 语义映射到原生事件循环的 post()。
///
/// @tparam Context  实现了 `void post(std::function<void()>)` 的执行上下文类型。
template <typename Context>
struct PostingScheduler {
	/// 指向所属执行上下文的原始指针；生命周期由 Context 实例负责。
	Context* ctx;

	bool operator==(const PostingScheduler&) const noexcept = default;

	// -------------------------------------------------------------------------
	// Sender — schedule() 的返回类型，代表"在 ctx 上调度一次执行"的异步操作。
	// -------------------------------------------------------------------------
	struct Sender {
		/// 标记为 stdexec sender（P2300 新风格标记）。
		using sender_concept = stdexec::sender_t;

		/// 本 sender 仅产生一个无值完成信号（set_value()，无参数）。
		using completion_signatures =
		    stdexec::completion_signatures<stdexec::set_value_t()>;

		/// 指向所属执行上下文。
		Context* ctx;

		// ---------------------------------------------------------------------
		// Operation state — connect() 的返回类型，持有 receiver 直到 start()。
		// ---------------------------------------------------------------------
		template <stdexec::receiver_of<completion_signatures> Rcvr>
		struct Op {
			/// 目标执行上下文。
			Context* ctx;

			/// 下游接收器，start() 执行完毕后通过它传递完成信号。
			Rcvr rcvr;

			/// @brief 启动操作：将"唤醒 receiver"的任务 post 到 ctx。
			/// @note  noexcept 保证：post() 内部异常通过 set_error 传递，不上浮。
			friend void tag_invoke(stdexec::start_t, Op& op) noexcept {
				try {
					op.ctx->post([rcvr = std::move(op.rcvr)]() mutable {
						stdexec::set_value(std::move(rcvr));
					});
				} catch (...) {
					stdexec::set_error(std::move(op.rcvr),
					                   std::current_exception());
				}
			}
		};

		/// @brief 将 sender 与 receiver 连接，返回 operation state。
		template <stdexec::receiver_of<completion_signatures> Rcvr>
		friend auto tag_invoke(stdexec::connect_t, Sender s, Rcvr r) {
			return Op<Rcvr>{s.ctx, std::move(r)};
		}

		/// @brief 向 stdexec 声明本 sender 完成时的归属 scheduler。
		///
		/// stdexec 的 continues_on 优化依赖此查询；若省略，exec::task 的
		/// "sticky 调度器"特性将无法识别当前执行上下文，可能产生额外的调度跳转。
		friend PostingScheduler
		tag_invoke(stdexec::get_completion_scheduler_t<stdexec::set_value_t>,
		           const Sender& s) noexcept {
			return PostingScheduler{s.ctx};
		}
	};

	/// @brief 返回代表"在 ctx 上调度一次执行"的 sender。
	Sender schedule() const noexcept { return Sender{ctx}; }
};

} // namespace yi::coro::detail
