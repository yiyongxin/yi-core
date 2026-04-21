#pragma once
#ifdef YI_HAS_CPPRESTSDK

// pplx::task<T> → stdexec sender 桥接（内部实现细节）
//
// 将 cpprestsdk 的异步任务适配为满足 stdexec sender 概念的类型，
// 使 yi::Task<T> 协程可通过 co_await 直接消费 pplx 异步结果。
//
// 约束：
//   - 调用 start() 后操作状态必须保持存活，直到 set_value/set_error 被调用。
//   - 不设置 get_completion_scheduler，由 exec::task 的 sticky 调度器决定恢复位置。

#include <pplx/pplxtasks.h>
#include <stdexec/execution.hpp>

#include <exception>
#include <type_traits>

namespace yi::http::detail {

template <typename T>
using pplx_completions_t = std::conditional_t<
    std::is_void_v<T>,
    stdexec::completion_signatures<stdexec::set_value_t(),
                                   stdexec::set_error_t(std::exception_ptr)>,
    stdexec::completion_signatures<stdexec::set_value_t(T),
                                   stdexec::set_error_t(std::exception_ptr)>>;

/// @brief 包装 pplx::task<T> 为 stdexec sender。
///
/// connect() 返回 Op；start() 将结果通过 receiver 的完成信号送出。
/// 若 pplx 任务抛出异常，转为 set_error(exception_ptr)。
template <typename T>
struct PplxSender {
    using sender_concept        = stdexec::sender_t;
    using completion_signatures = pplx_completions_t<T>;

    pplx::task<T> task_;

    template <typename Rcvr>
    struct Op {
        pplx::task<T> task_;
        Rcvr          rcvr_;

        friend void tag_invoke(stdexec::start_t, Op& op) noexcept {
            try {
                op.task_.then([rcvr = std::move(op.rcvr_)](pplx::task<T> t) mutable {
                    try {
                        if constexpr (std::is_void_v<T>) {
                            t.get();
                            stdexec::set_value(std::move(rcvr));
                        } else {
                            stdexec::set_value(std::move(rcvr), t.get());
                        }
                    } catch (...) {
                        stdexec::set_error(std::move(rcvr), std::current_exception());
                    }
                });
            } catch (...) {
                stdexec::set_error(std::move(op.rcvr_), std::current_exception());
            }
        }
    };

    template <stdexec::receiver Rcvr>
    friend auto tag_invoke(stdexec::connect_t, PplxSender s, Rcvr r) {
        return Op<Rcvr>{std::move(s.task_), std::move(r)};
    }
};

template <typename T>
PplxSender<T> to_sender(pplx::task<T> t) {
    return PplxSender<T>{std::move(t)};
}

} // namespace yi::http::detail

#endif // YI_HAS_CPPRESTSDK
