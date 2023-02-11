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
#ifndef HTTP_HANDLERS_HPP
#define HTTP_HANDLERS_HPP

#include "../json.hpp"
#include "../util.hpp"
#include "../virt/connection.hpp"
#include "../virt/util.hpp"
#include "router.hpp"
#include <boost/asio/buffer.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http/dynamic_body.hpp>
#include <sys/wait.h>
#include <thread>

namespace webvirt::http
{

template <typename client_t>
void on_connect(client_t &)
{
}

template <typename connection_t>
void on_accept(connection_t &)
{
}

void on_close();

void on_error(const char *, boost::beast::error_code);

template <typename connection_t>
void on_request(
    connection_t &,
    const boost::beast::http::request<boost::beast::http::dynamic_body> &,
    boost::beast::http::response<boost::beast::http::string_body> &)
{
}

void on_response(
    const boost::beast::http::response<boost::beast::http::string_body> &);

}; // namespace webvirt::http

#endif /* HTTP_HANDLERS_HPP */
