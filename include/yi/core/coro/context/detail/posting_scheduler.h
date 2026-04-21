#pragma once

// 通用 posting 调度器（内部实现细节）
//
// 将任何实现了 post(std::function<void()>) 的上下文适配为 stdexec::scheduler。
// 采用 tag_invoke 扩展点，兼容 P2300 / stdexec 主线版本。

#include <stdexec/execution.hpp>
#include <exception>

namespace yi::coro::detail
{

template <typename Context>
struct PostingScheduler
{
	Context* ctx;

	bool operator==(const PostingScheduler&) const noexcept = default;

	struct Sender
	{
		using sender_concept        = stdexec::sender_t;
		using completion_signatures = stdexec::completion_signatures<stdexec::set_value_t()>;

		Context* ctx;

		template <stdexec::receiver_of<completion_signatures> Rcvr>
		struct Op {
			Context* ctx;
			Rcvr     rcvr;

			friend void tag_invoke(stdexec::start_t, Op& op) noexcept 
			{
				try
				{
					op.ctx->post([rcvr = std::move(op.rcvr)]() mutable {
						stdexec::set_value(std::move(rcvr));
					});
				}
				catch (...)
				{
					stdexec::set_error(std::move(op.rcvr), std::current_exception());
				}
			}
		};

		template <stdexec::receiver_of<completion_signatures> Rcvr>
		friend auto tag_invoke(stdexec::connect_t, Sender s, Rcvr r)
		{
			return Op<Rcvr>{s.ctx, std::move(r)};
		}

		// exec::task 的 sticky 调度器优化依赖此查询；省略会导致额外的调度跳转。
		friend PostingScheduler
		tag_invoke(stdexec::get_completion_scheduler_t<stdexec::set_value_t>, const Sender& s) noexcept
		{
			return PostingScheduler{s.ctx};
		}
	};

	// 返回代表"在 ctx 上调度一次执行"的 sender。
	Sender schedule() const noexcept { return Sender{ctx}; }
};

} // namespace yi::coro::detail
