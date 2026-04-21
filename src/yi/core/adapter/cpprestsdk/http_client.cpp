#ifdef YI_HAS_CPPRESTSDK

#include <yi/core/adapter/cpprestsdk/http_client.h>

#include <stdexec/execution.hpp>

namespace yi::http {

HttpClient::HttpClient(utility::string_t base_uri)
    : client_(std::move(base_uri))
    , owned_ctx_(std::make_unique<yi::coro::PplxContext>())
    , ctx_(owned_ctx_.get()) {}

HttpClient::HttpClient(utility::string_t base_uri, web::http::client::http_client_config config)
    : client_(std::move(base_uri), std::move(config))
    , owned_ctx_(std::make_unique<yi::coro::PplxContext>())
    , ctx_(owned_ctx_.get()) {}

HttpClient::HttpClient(utility::string_t base_uri, yi::coro::PplxContext& ctx)
    : client_(std::move(base_uri))
    , ctx_(&ctx) {}

HttpClient::HttpClient(utility::string_t base_uri, web::http::client::http_client_config config,
                       yi::coro::PplxContext& ctx)
    : client_(std::move(base_uri), std::move(config))
    , ctx_(&ctx) {}

yi::Task<web::http::http_response> HttpClient::request(web::http::http_request req) {
    // client_.request() 立即启动 HTTP 操作（由 pplx 内部线程池驱动）。
    // starts_on 确保 PplxSender::start() 在 ctx_ 调度器上执行，
    // 之后 exec::task 的 sticky 调度器负责将协程恢复到正确线程。
    co_return co_await stdexec::starts_on(
        ctx_->scheduler(),
        detail::to_sender(client_.request(std::move(req))));
}

yi::Task<web::http::http_response> HttpClient::request(web::http::method method,
                                                         utility::string_t path) {
    co_return co_await request(web::http::http_request{method, std::move(path)});
}

yi::Task<web::http::http_response> HttpClient::request(web::http::method method,
                                                         utility::string_t path,
                                                         web::json::value  body) {
    web::http::http_request req{method, std::move(path)};
    req.set_body(std::move(body));
    co_return co_await request(std::move(req));
}

} // namespace yi::http

#endif // YI_HAS_CPPRESTSDK
