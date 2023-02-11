/*
 * Copyright 2023 Kevin Morris
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */
#include "middleware.hpp"
#include "../json.hpp"
#include "../syscaller.hpp"
#include "../virt/util.hpp"
#include "util.hpp"
using namespace webvirt;
using namespace http;

static bool allowed_methods(const std::vector<beast::http::verb> &methods,
                            beast::http::verb method)
{
    auto it = std::find(methods.begin(), methods.end(), method);
    return it != methods.end();
}

http::route_function
middleware::with_methods(const std::vector<boost::beast::http::verb> &methods,
                         http::route_function route_fn)
{
    return [methods,
            route_fn](const auto &m, const auto &request, auto &response) {
        // TODO: Check + response for OPTIONS here

        if (!allowed_methods(methods, request.method())) {
            return response.result(beast::http::status::method_not_allowed);
        }

        return route_fn(m, request, response);
    };
}

http::route_function middleware::with_libvirt_domain(
    std::function<void(virt::connection &, virt::domain domain,
                       const std::smatch &, const http::request &,
                       http::response &)>
        route_fn)
{
    return with_libvirt([route_fn](virt::connection &conn,
                                   const std::smatch &match,
                                   const http::request &request,
                                   http::response &response) {
        const std::string name(match[2]);
        virt::domain domain;
        try {
            domain = conn.domain(name);
        } catch (const std::domain_error &) {
            auto error = json::error("Domain not found");
            return http::set_response(response,
                                      json::stringify(error),
                                      beast::http::status::not_found);
        }

        return route_fn(conn, domain, match, request, response);
    });
}

http::route_function middleware::with_libvirt(
    std::function<void(virt::connection &, const std::smatch &,
                       const http::request &, http::response &)>
        route_fn)
{
    return with_user([route_fn](const auto &match,
                                const auto &request,
                                auto &response) {
        const std::string user = match[1];
        virt::connection conn;
        try {
            conn.connect(virt::uri(user));
        } catch (const std::runtime_error &e) {
            auto error = json::error("Unable to connect to libvirt");
            return set_response(response,
                                json::stringify(error),
                                beast::http::status::internal_server_error);
        }

        response.set("Content-Type", "application/json");
        response.result(beast::http::status::ok);
        return route_fn(conn, match, request, response);
    });
}

http::route_function middleware::with_user(route_function route_fn)
{
    return [route_fn](const auto &match, const auto &request, auto &response) {
        // Parse expected JSON from the request body.
        std::string user(match[1]);

        auto &sys = syscaller::instance();
        auto *passwd = sys.getpwnam(user.c_str());
        if (!passwd) {
            auto error = json::error("Unable to locate user");
            return set_response(response,
                                json::stringify(error),
                                beast::http::status::not_found);
        }

        return route_fn(match, request, response);
    };
}
