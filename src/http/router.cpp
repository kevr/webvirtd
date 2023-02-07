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
#include "../syscaller.hpp"
#include "../virt/util.hpp"
#include "middleware.hpp"
#include <fmt/format.h>
#include <iostream>
#include <regex>
#include <vector>
using namespace webvirt;

void http::router::run(const request_t &request, response_t &response)
{
    const auto request_uri = std::string(request.target());
    const auto method = std::string(request.method_string());

    std::function<void(const std::string &)> log([&](const auto &m) {
        log_.info(m);
    });

    std::function<void()> next = [&response] {
        Json::Value data(Json::objectValue);
        data["detail"] = "Not Found";
        response.body().append(json::stringify(data));
        response.content_length(response.body().size());
        response.result(beast::http::status::not_found);
    };

    for (auto &route_ : routes_) {
        const auto &re = regex_[route_.first];
        std::smatch match;
        if (std::regex_match(request_uri, match, re)) {
            next = [&, match] {
                route_.second(match, request, response);
            };
            break;
        }
    }

    next();

    if (response.result() != boost::beast::http::status::ok &&
        response.result() != boost::beast::http::status::created &&
        response.result() != boost::beast::http::status::temporary_redirect) {
        log = [&](const auto &message) {
            log_.error(message);
        };
    }

    auto major = response.version() / 10;
    auto minor = response.version() % 10;
    log(fmt::format("\"{} {} HTTP/{}.{}\" {} {}",
                    method,
                    request_uri,
                    major,
                    minor,
                    response.result_int(),
                    response.body().size()));
}

void http::router::route(
    std::string request_uri,
    std::function<void(const std::smatch &, const request_t &, response_t &)>
        fn)
{
    routes_[request_uri] = fn;
    regex_[request_uri] = request_uri;
}
