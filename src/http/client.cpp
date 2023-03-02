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
#include <http/client.hpp>

using namespace webvirt;
using namespace http;

client::client(io_context &io, std::string socket_path, std::string host,
               int version)
    : socket_path_(std::move(socket_path))
    , io_(io)
    , socket_(boost::beast::net::make_strand(io_))
{
    this->host(std::move(host)).version(version);
}

client &client::host(std::string value)
{
    host_ = std::move(value);
    return *this;
}

const std::string &client::host() const
{
    return host_;
}

client &client::version(int version)
{
    version_ = version;
    return *this;
}

int client::version() const
{
    return version_;
}

client &client::async_options(const char *target)
{
    init_request(target);
    request_.method(beast::http::verb::options);
    async_connect();
    return *this;
}

client &client::async_get(const char *target)
{
    init_request(target);
    request_.method(beast::http::verb::get);
    async_connect();
    return *this;
}

client &client::async_post(const char *target, const std::string &data)
{
    init_request(target);
    request_.method(beast::http::verb::post);
    request_.body().append(data);
    request_.content_length(request_.body().size());
    async_connect();
    return *this;
}

const beast::http::request<beast::http::string_body> &client::request() const
{
    return request_;
}

void client::close()
{
    socket_.close();
}

std::size_t client::run()
{
    std::size_t count = io_.run();
    close();
    return count;
}

void client::init_request(const char *target)
{
    request_.version(version_);
    request_.target(target);
    request_.set(beast::http::field::host, host_);
    request_.set(beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    request_.set(beast::http::field::accept, "*/*");
}

void client::async_connect()
{
    io_.restart();
    socket_.async_connect(
        socket_path_,
        boost::beast::bind_front_handler(&client::client_on_connect,
                                         this->shared_from_this()));
}

void client::client_on_connect(boost::beast::error_code ec)
{
    CLASS_TRACE("Connecting");

    if (ec) {
        CLASS_ETRACE(ec.message());
        return on_error_(__func__, ec);
    }
    on_connect_(*this);

    beast::http::async_write(
        socket_,
        request_,
        boost::beast::bind_front_handler(&client::client_on_write,
                                         this->shared_from_this()));
}

void client::client_on_write(boost::beast::error_code ec, std::size_t bytes)
{
    boost::ignore_unused(bytes);

    if (ec) {
        CLASS_ETRACE(ec.message());
        return on_error_(__func__, ec);
    }

    beast::http::async_read(
        socket_,
        buffer_,
        response_,
        boost::beast::bind_front_handler(&client::client_on_read,
                                         this->shared_from_this()));
}

void client::client_on_read(boost::beast::error_code ec, std::size_t bytes)
{
    boost::ignore_unused(bytes);

    if (ec) {
        CLASS_ETRACE(ec.message());
        return on_error_(__func__, ec);
    }
    on_response_(response_);

    socket_.shutdown(net::unix::socket::shutdown_both, ec);
    on_close_();
}
