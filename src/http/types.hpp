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
#ifndef HTTP_TYPES_HPP
#define HTTP_TYPES_HPP

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <regex>

namespace webvirt
{

namespace net
{

using unix = boost::asio::local::stream_protocol;
using tcp = boost::asio::ip::tcp;

}; // namespace net

namespace beast = boost::beast;

namespace http
{

enum version : int {
    http_1 = 10,
    http_1_1 = 11,
};

using request = beast::http::request<beast::http::dynamic_body>;
using response = beast::http::response<beast::http::string_body>;

using route_function = std::function<void(
    const std::smatch &, const http::request &, http::response &)>;

}; // namespace http

}; // namespace webvirt

#endif
