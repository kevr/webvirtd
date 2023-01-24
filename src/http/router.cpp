#include "router.hpp"
#include <vector>
using namespace webvirt;

bool http::allowed_methods(const std::vector<beast::http::verb> &methods,
                           beast::http::verb method)
{
    auto it = std::find(methods.begin(), methods.end(), method);
    return it != methods.end();
}

http::router::router(const request_t &request, response_t &response)
    : request_(request)
    , response_(response)
{
    route("/", [](const auto &request, auto &response) {
        if (allowed_methods(
                { beast::http::verb::get, beast::http::verb::post },
                request.method())) {
            response.result(beast::http::status::ok);
        } else {
            response.result(beast::http::status::method_not_allowed);
        }
    });
}

void http::router::run()
{
    const std::string request_uri = request_.target().to_string();
    auto it = routes_.find(request_uri);
    if (it == routes_.end()) {
        response_.result(beast::http::status::not_found);
    } else {
        auto route = it->second;
        route(request_, response_);
    }
}

void http::router::route(
    std::string request_uri,
    std::function<void(const request_t &, response_t &)> fn)
{
    routes_[request_uri] = fn;
}
