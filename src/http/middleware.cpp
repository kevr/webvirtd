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
#include <http/middleware.hpp>
#include <http/util.hpp>
#include <syscall.hpp>
#include <util/json.hpp>
#include <virt/util.hpp>

#include <chrono>
#include <regex>
#include <sstream>

using namespace webvirt;
using namespace http;
using namespace middleware;

static bool allowed_methods(const std::vector<beast::http::verb> &methods,
                            beast::http::verb method)
{
    auto it = std::find(methods.begin(), methods.end(), method);
    return it != methods.end();
}

http_route_function
middleware::with_methods(const std::vector<boost::beast::http::verb> &methods,
                         http_route_function route_fn)
{
    return [methods, route_fn](http::connection_ptr conn,
                               const std::smatch &match,
                               const http::request &request,
                               http::response &response) {
        // Construt and add the Allow header.
        std::string allow;
        for (auto method : methods) {
            std::stringstream ss;
            ss << method;
            allow.append(ss.str() + ", ");
        }
        allow.append("OPTIONS");
        response.set(beast::http::field::allow, allow);

        if (request.method() == beast::http::verb::options) {
            // On OPTIONS, no-op and respond with an OK + headers.
            return;
        }

        if (!allowed_methods(methods, request.method())) {
            return response.result(beast::http::status::method_not_allowed);
        }

        return route_fn(std::move(conn), match, request, response);
    };
}

http_route_function middleware::with_libvirt_domain(
    virt::connection_pool &pool,
    std::function<void(virt::connection &, virt::domain domain,
                       http::connection_ptr, const std::smatch &,
                       const http::request &, http::response &)>
        route_fn)
{
    return with_libvirt(
        pool,
        [route_fn](virt::connection &conn,
                   http::connection_ptr http_conn,
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

            return route_fn(
                conn, domain, std::move(http_conn), match, request, response);
        });
}

http_route_function middleware::with_libvirt(
    virt::connection_pool &pool,
    std::function<void(virt::connection &, http::connection_ptr,
                       const std::smatch &, const http::request &,
                       http::response &)>
        route_fn)
{
    return with_user([&pool, route_fn](http::connection_ptr http_conn,
                                       const std::smatch &match,
                                       const http::request &request,
                                       http::response &response) {
        const std::string user = match[1];

        virt::connection *conn;
        try {
            conn = &pool.get(user);
        } catch (const std::runtime_error &e) {
            auto error = json::error("Unable to connect to libvirt");
            return set_response(response,
                                json::stringify(error),
                                beast::http::status::internal_server_error);
        }

        return route_fn(*conn, std::move(http_conn), match, request, response);
    });
}

http_route_function middleware::with_user(http_route_function route_fn)
{
    return [route_fn](http::connection_ptr conn,
                      const std::smatch &match,
                      const http::request &request,
                      http::response &response) {
        // Parse expected JSON from the request body.
        std::string user(match[1]);

        auto &sys = syscall::ref();
        auto *passwd = sys.getpwnam(user.c_str());
        if (!passwd) {
            auto error = json::error("Unable to locate user");
            return set_response(response,
                                json::stringify(error),
                                beast::http::status::not_found);
        }

        return route_fn(std::move(conn), match, request, response);
    };
}
