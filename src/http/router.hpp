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
#ifndef HTTP_ROUTER_HPP
#define HTTP_ROUTER_HPP

#include "../logging.hpp"
#include "../virt/connection.hpp"
#include "namespaces.hpp"
#include <boost/beast.hpp>
#include <map>
#include <regex>
#include <vector>

namespace webvirt::http
{

class router
{
private:
    logger log_;

    using request_t = beast::http::request<beast::http::dynamic_body>;
    using response_t = beast::http::response<beast::http::string_body>;

    using route_function_t = std::function<void(
        const std::smatch &, const request_t &, response_t &)>;
    std::map<std::string, route_function_t> routes_;

public:
    void run(const request_t &request, response_t &response);
    void route(std::string request_uri,
               std::function<void(const std::smatch &, const request_t &,
                                  response_t &)>);
};

}; // namespace webvirt::http

#endif /* HTTP_ROUTER_HPP */
