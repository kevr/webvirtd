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
using namespace webvirt;
using namespace http;

static bool allowed_methods(const std::vector<beast::http::verb> &methods,
                            beast::http::verb method)
{
    auto it = std::find(methods.begin(), methods.end(), method);
    return it != methods.end();
}

std::function<void(const std::smatch &, const http::request &,
                   http::response &)>
middleware::with_methods(
    const std::vector<boost::beast::http::verb> &methods,
    std::function<void(const std::smatch &, const http::request &,
                       http::response &)>
        route_fn)
{
    return [methods,
            route_fn](const auto &m, const auto &request, auto &response) {
        if (!allowed_methods(methods, request.method())) {
            return response.result(beast::http::status::method_not_allowed);
        }

        return route_fn(m, request, response);
    };
}

std::function<void(const std::smatch &, const http::request &,
                   http::response &)>
middleware::with_libvirt(
    std::function<void(virt::connection &, const std::string &,
                       const std::smatch &, const http::request &,
                       http::response &)>
        route_fn)
{
    return with_user([route_fn](const auto &user,
                                const auto &m,
                                const auto &request,
                                auto &response) {
        virt::connection conn;
        Json::Value json(Json::objectValue);
        try {
            conn.connect(virt::uri(user));
        } catch (const std::runtime_error &e) {
            json["detail"] = "Unable to connect to libvirt";
            response.result(beast::http::status::internal_server_error);
            auto output = json::stringify(json);
            response.body().append(output);
            response.content_length(response.body().size());
            return;
        }

        return route_fn(conn, user, m, request, response);
    });
}

std::function<void(const std::smatch &, const http::request &,
                   http::response &)>
middleware::with_user(
    std::function<void(const std::string &, const std::smatch &,
                       const http::request &, http::response &)>
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
