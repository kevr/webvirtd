#include "router.hpp"
#include "../json.hpp"
#include "../syscaller.hpp"
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

std::function<
    void(const std::smatch &,
         const boost::beast::http::request<boost::beast::http::dynamic_body> &,
         boost::beast::http::response<boost::beast::http::string_body> &)>
http::router::with_methods(
    const std::vector<boost::beast::http::verb> &methods,
    std::function<void(
        const std::smatch &,
        const boost::beast::http::request<boost::beast::http::dynamic_body> &,
        boost::beast::http::response<boost::beast::http::string_body> &)>
        route_fn)
{
    return [methods,
            route_fn](const auto &m, const auto &request, auto &response) {
        if (!http::allowed_methods(methods, request.method())) {
            return response.result(beast::http::status::method_not_allowed);
        }

        return route_fn(m, request, response);
    };
}

std::function<
    void(const std::smatch &,
         const boost::beast::http::request<boost::beast::http::dynamic_body> &,
         boost::beast::http::response<boost::beast::http::string_body> &)>
http::router::with_user(
    std::function<void(
        const std::string &, const std::smatch &,
        const boost::beast::http::request<boost::beast::http::dynamic_body> &,
        boost::beast::http::response<boost::beast::http::string_body> &)>
        route_fn)
{
    return [route_fn](const auto &m, const auto &request, auto &response) {
        // Parse expected JSON from the request body.
        Json::Value root;
        Json::Value error(Json::objectValue);
        try {
            root = json::parse(request.body());
        } catch (const std::invalid_argument &e) {
            error["detail"] = "Unable to parse request body JSON";
            response.body().append(json::stringify(error));
            return response.result(beast::http::status::bad_request);
        }

        auto &sys = syscaller::instance();
        auto user = root.get("user", "").asString();
        auto *passwd = sys.getpwnam(user.c_str());
        if (!passwd) {
            error["detail"] = "Unable to locate user";
            response.body().append(json::stringify(error));
            return response.result(beast::http::status::not_found);
        }

        return route_fn(user, m, request, response);
    };
}
