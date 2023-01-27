#include "router.hpp"
#include <regex>
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
    route("/", [](const auto &, const auto &request, auto &response) {
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

    for (auto &route_ : routes_) {
        const std::regex re(route_.first);
        std::smatch match;
        if (std::regex_match(request_uri, match, re)) {
            return route_.second(match, request_, response_);
        }
    }

    return response_.result(beast::http::status::not_found);
}

void http::router::route(
    std::string request_uri,
    std::function<void(const std::smatch &, const request_t &, response_t &)>
        fn)
{
    routes_[request_uri] = fn;
}
