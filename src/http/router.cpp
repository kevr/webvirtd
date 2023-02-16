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
#include "router.hpp"
#include "../json.hpp"
#include "../logging.hpp"
#include "../syscaller.hpp"
#include "../virt/util.hpp"
#include "middleware.hpp"
#include "util.hpp"
#include <chrono>
#include <fmt/format.h>
#include <iostream>
#include <regex>
#include <vector>
using namespace webvirt;

void http::router::run(const http::request &request, http::response &response)
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

    std::chrono::high_resolution_clock clock;
    std::chrono::high_resolution_clock::time_point start;
    std::chrono::high_resolution_clock::time_point end;

    for (auto &route : routes_) {
        const std::regex &re = regex_.at(route.first);
        std::smatch match;
        if (std::regex_match(request_uri, match, re)) {
            next = [&, match] {
                start = clock.now();
                route.second(match, request, response);
                end = clock.now();
            };
            break;
        }
    }

    next();

    int status_code = response.result_int();
    if (!(status_code >= 200 && status_code < 400)) {
        log = [](const auto &message) {
            logger::error(message);
        };
    }

    auto elapsed = std::chrono::duration<double>(end - start).count() * 1000;
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
                         http::route_function fn)
{
    routes_[request_uri] = fn;
    regex_[request_uri] = request_uri;
}
