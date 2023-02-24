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
#ifndef HTTP_MIDDLEWARE_HPP
#define HTTP_MIDDLEWARE_HPP

#include <http/connection.hpp>
#include <http/types.hpp>
#include <virt/connection.hpp>
#include <virt/connection_pool.hpp>
#include <virt/domain.hpp>
#include <ws/connection.hpp>

#include <regex>

namespace webvirt::http
{

namespace middleware
{

using http_route_function = connection::route_function;

http_route_function with_methods(const std::vector<boost::beast::http::verb> &,
                                 http_route_function);

http_route_function with_libvirt_domain(
    virt::connection_pool &pool,
    std::function<void(virt::connection &, virt::domain domain,
                       http::connection_ptr, const std::smatch &,
                       const http::request &, http::response &)>);

http_route_function
with_libvirt(virt::connection_pool &,
             std::function<void(virt::connection &, http::connection_ptr,
                                const std::smatch &, const http::request &,
                                http::response &)>);

http_route_function with_user(http_route_function);

}; // namespace middleware

}; // namespace webvirt::http

#endif /* HTTP_MIDDLEWARE_HPP */
