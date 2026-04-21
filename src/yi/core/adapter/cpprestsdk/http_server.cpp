#ifdef YI_HAS_CPPRESTSDK

#include <yi/core/adapter/cpprestsdk/http_server.h>

#include <exec/start_detached.hpp>
#include <stdexec/execution.hpp>

namespace yi::http {

HttpServer::HttpServer(utility::string_t uri)
    : listener_(std::move(uri))
    , owned_ctx_(std::make_unique<yi::coro::PplxContext>())
    , ctx_(owned_ctx_.get()) {}

HttpServer::HttpServer(utility::string_t                                           uri,
                       web::http::experimental::listener::http_listener_config config)
    : listener_(std::move(uri), std::move(config))
    , owned_ctx_(std::make_unique<yi::coro::PplxContext>())
    , ctx_(owned_ctx_.get()) {}

HttpServer::HttpServer(utility::string_t uri, yi::coro::PplxContext& ctx)
    : listener_(std::move(uri))
    , ctx_(&ctx) {}

HttpServer::HttpServer(utility::string_t                                           uri,
                       web::http::experimental::listener::http_listener_config config,
                       yi::coro::PplxContext&                                      ctx)
    : listener_(std::move(uri), std::move(config))
    , ctx_(&ctx) {}

void HttpServer::on(web::http::method method, Handler handler) {
    // cpprestsdk 在 pplx 线程上回调；dispatch 负责将协程跳转到 ctx_。
    listener_.support(method, [this, h = std::move(handler)](web::http::http_request req) {
        dispatch(std::move(req), h);
    });
}

void HttpServer::dispatch(web::http::http_request req, Handler handler) {
    // fire-and-forget：在 ctx_ 调度器上启动处理器协程，不等待其完成。
    exec::start_detached(
        stdexec::starts_on(ctx_->scheduler(), handler(std::move(req))));
}

yi::Task<void> HttpServer::open() {
    co_await stdexec::starts_on(ctx_->scheduler(),
                                detail::to_sender(listener_.open()));
}

yi::Task<void> HttpServer::close() {
    co_await stdexec::starts_on(ctx_->scheduler(),
                                detail::to_sender(listener_.close()));
}

} // namespace yi::http

#endif // YI_HAS_CPPRESTSDK
