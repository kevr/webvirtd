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

#include "../virt/connection.hpp"
#include "namespaces.hpp"
#include <regex>

namespace webvirt::http::middleware
{

std::function<void(const std::smatch &, const http::request &,
                   http::response &)>
with_methods(const std::vector<boost::beast::http::verb> &,
             std::function<void(const std::smatch &, const http::request &,
                                http::response &)>);

std::function<void(const std::smatch &, const http::request &,
                   http::response &)>
    with_libvirt(std::function<void(virt::connection &, const std::string &,
                                    const std::smatch &, const http::request &,
                                    http::response &)>);

std::function<void(const std::smatch &, const http::request &,
                   http::response &)>
    with_user(std::function<void(const std::string &, const std::smatch &,
                                 const http::request &, http::response &)>);

}; // namespace webvirt::http::middleware

#endif /* HTTP_MIDDLEWARE_HPP */
