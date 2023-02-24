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
#include <data/host.hpp>
#include <http/util.hpp>
#include <util/json.hpp>
#include <util/logging.hpp>
#include <views/host.hpp>

using namespace webvirt::views;

void host::show(virt::connection &conn, http::connection_ptr,
                const std::smatch &, const http::request &,
                http::response &response)
{
    return http::set_response(
        response, data::host(conn), beast::http::status::ok);
}

void host::networks(virt::connection &conn, http::connection_ptr,
                    const std::smatch &, const http::request &,
                    http::response &response)
{
    return http::set_response(
        response, data::networks(conn), beast::http::status::ok);
}
