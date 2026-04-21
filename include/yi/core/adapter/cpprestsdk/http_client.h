#pragma once
#ifdef YI_HAS_CPPRESTSDK

#include "detail/pplx_sender.h"

#include <yi/core/coro/context/pplx_context.h>
#include <yi/core/coro/task.h>

#include <cpprest/http_client.h>
#include <cpprest/json.h>

#include <memory>

namespace yi::http {

/// @brief 协程版 HTTP 客户端，封装 web::http::client::http_client。
///
/// 所有请求方法返回 yi::Task<web::http::http_response>，可在协程内 co_await。
///
/// 执行上下文：
///   - 无参/配置参数构造：使用内部默认 PplxContext（pplx 全局线程池）。
///   - 传入 ctx 参数构造：请求完成后在 ctx 所在调度器上恢复协程。
///
/// 注：HttpClient 不可拷贝/移动，通常以 unique_ptr 或成员变量持有。
class HttpClient {
public:
    /// 使用默认 pplx 上下文。
    explicit HttpClient(utility::string_t base_uri);

    /// 使用自定义 http_client_config，默认 pplx 上下文。
    HttpClient(utility::string_t base_uri, web::http::client::http_client_config config);

    /// 使用调用方提供的 PplxContext；ctx 生命周期须覆盖所有 co_await 调用。
    HttpClient(utility::string_t base_uri, yi::coro::PplxContext& ctx);

    /// 同上，附带自定义配置。
    HttpClient(utility::string_t base_uri, web::http::client::http_client_config config,
               yi::coro::PplxContext& ctx);

    HttpClient(const HttpClient&)            = delete;
    HttpClient& operator=(const HttpClient&) = delete;
    HttpClient(HttpClient&&)                 = delete;
    HttpClient& operator=(HttpClient&&)      = delete;

    ~HttpClient() = default;

    /// @brief 发送任意 HTTP 请求，co_await 获取响应。
    yi::Task<web::http::http_response> request(web::http::http_request req);

    /// @brief 发送指定方法的请求（无请求体）。
    yi::Task<web::http::http_response> request(web::http::method      method,
                                                utility::string_t      path = {});

    /// @brief 发送指定方法的请求（带 JSON 请求体）。
    yi::Task<web::http::http_response> request(web::http::method      method,
                                                utility::string_t      path,
                                                web::json::value       body);

private:
    web::http::client::http_client        client_;
    std::unique_ptr<yi::coro::PplxContext> owned_ctx_;  // 无用户传入 ctx 时使用
    yi::coro::PplxContext*                 ctx_;         // 始终有效
};

} // namespace yi::http

#endif // YI_HAS_CPPRESTSDK
