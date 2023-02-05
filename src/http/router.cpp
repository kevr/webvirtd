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
#include <iostream>
#include <regex>
#include <vector>
using namespace webvirt;

void http::router::run(const request_t &request, response_t &response)
{
    const std::string request_uri = request.target().to_string();

    for (auto &route_ : routes_) {
        const std::regex re(route_.first);
        std::smatch match;
        if (std::regex_match(request_uri, match, re)) {
            return route_.second(match, request, response);
        }
    }

    return response.result(beast::http::status::not_found);
}

void http::router::route(
    std::string request_uri,
    std::function<void(const std::smatch &, const request_t &, response_t &)>
        fn)
{
    routes_[request_uri] = fn;
}
