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
#include <http/router.hpp>
#include <http/util.hpp>
#include <syscall.hpp>
#include <util/bench.hpp>
#include <util/json.hpp>
#include <util/logging.hpp>
#include <util/retry.hpp>
#include <virt/util.hpp>

#include <chrono>
#include <fmt/format.h>
#include <iostream>
#include <regex>
#include <vector>

using namespace webvirt;

void http::router::run(http::connection_ptr http_conn,
                       const http::request &request, http::response &response)
{
    const auto request_uri = std::string(request.target());
    const auto method = std::string(request.method_string());

    response.set(beast::http::field::content_type, "application/json");
    response.result(beast::http::status::ok);

    std::function<void(const std::string &)> log([](const auto &message) {
        logger::info(message);
    });

    std::function<void()> next = [&request, &response] {
        Json::Value data(Json::objectValue);
        data["detail"] = "Not Found";
        std::string res;
        if (request.method() != beast::http::verb::options) {
            res = json::stringify(json::error("Not Found"));
        }
        set_response(response, res, beast::http::status::not_found);
    };

    bench<double> bench_;
    for (auto &route : routes_) {
        const std::regex &re = regex_.at(route.first);
        std::smatch match;
        if (std::regex_match(request_uri, match, re)) {
            next = [&, match] {
                bench_.start();

                try {
                    // Try route.second five times.
                    retry([&, match] {
                        response = http::response();
                        route.second(
                            std::move(http_conn), match, request, response);
                    }).retries(5)();
                } catch (std::exception &exc) {
                    // If it fails the sixth time, catch its std::domain_error.
                    http::set_response(
                        response,
                        json::error(exc.what()),
                        beast::http::status::internal_server_error);
                }

                bench_.end();
            };
            break;
        }
    }

    next();

    int status_code = response.result_int();
    if (status_code >= 400) {
        log = [](const auto &message) {
            logger::error(message);
        };
    }

    double elapsed = bench_.elapsed() * 1000;
    auto major = response.version() / 10;
    auto minor = response.version() % 10;
    log(fmt::format("\"{} {} HTTP/{}.{}\" {} {} (took {}ms)",
                    method,
                    request_uri,
                    major,
                    minor,
                    response.result_int(),
                    response.body().size(),
                    int(elapsed)));
}

void http::router::route(const std::string &request_uri,
                         http::connection::route_function fn)
{
    routes_[request_uri] = fn;
    regex_[request_uri] = request_uri;
}
