#pragma once
#ifdef YI_HAS_CPPRESTSDK

#include "detail/pplx_sender.h"

#include <yi/core/coro/context/pplx_context.h>
#include <yi/core/coro/task.h>

#include <cpprest/http_listener.h>

#include <functional>
#include <memory>

namespace yi::http {

/// @brief 协程版 HTTP 服务端，封装 web::http::experimental::listener::http_listener。
///
/// 通过 on() 注册各 HTTP 方法的协程处理器；收到请求时在 ctx 上异步派发。
///
/// 执行上下文：
///   - 无 ctx 参数构造：处理器协程在内部默认 PplxContext 上运行。
///   - 传入 ctx 参数构造：处理器协程在 ctx 所在调度器上运行。
///
/// 典型用法：
/// @code
///   yi::http::HttpServer server(U("http://0.0.0.0:8080"));
///   server.on(web::http::methods::GET, [](web::http::http_request req) -> yi::Task<void> {
///       req.reply(web::http::status_codes::OK, U("hello"));
///       co_return;
///   });
///   co_await server.open();
/// @endcode
///
/// 注：HttpServer 不可拷贝/移动。
class HttpServer {
public:
    using Handler = std::function<yi::Task<void>(web::http::http_request)>;

    /// 使用默认 pplx 上下文。
    explicit HttpServer(utility::string_t uri);

    /// 使用自定义 http_listener_config，默认 pplx 上下文。
    HttpServer(utility::string_t                                           uri,
               web::http::experimental::listener::http_listener_config config);

    /// 使用调用方提供的 PplxContext；ctx 生命周期须覆盖所有请求处理。
    HttpServer(utility::string_t uri, yi::coro::PplxContext& ctx);

    /// 同上，附带自定义配置。
    HttpServer(utility::string_t                                           uri,
               web::http::experimental::listener::http_listener_config config,
               yi::coro::PplxContext&                                      ctx);

    HttpServer(const HttpServer&)            = delete;
    HttpServer& operator=(const HttpServer&) = delete;
    HttpServer(HttpServer&&)                 = delete;
    HttpServer& operator=(HttpServer&&)      = delete;

    ~HttpServer() = default;

    /// @brief 注册指定 HTTP 方法的协程处理器。
    ///
    /// 对同一方法多次调用 on() 以最后一次为准（cpprestsdk 语义）。
    void on(web::http::method method, Handler handler);

    /// @brief 启动监听，co_await 直至监听器就绪。
    yi::Task<void> open();

    /// @brief 停止监听，co_await 直至监听器关闭。
    yi::Task<void> close();

private:
    // 在 ctx_ 上派发一次请求到 handler 协程（fire-and-forget）。
    void dispatch(web::http::http_request req, Handler handler);

    web::http::experimental::listener::http_listener  listener_;
    std::unique_ptr<yi::coro::PplxContext>             owned_ctx_;
    yi::coro::PplxContext*                             ctx_;
};

} // namespace yi::http

#endif // YI_HAS_CPPRESTSDK
