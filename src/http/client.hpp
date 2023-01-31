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
#ifndef HTTP_CLIENT_HPP
#define HTTP_CLIENT_HPP

#include "../util.hpp"
#include "handlers.hpp"
#include "io_service.hpp"
#include "namespaces.hpp"
#include <boost/beast.hpp>
#include <iostream>
#include <memory>

namespace webvirt::http
{

template <typename protocol_t>
class client : public std::enable_shared_from_this<client<protocol_t>>
{
    std::string socket_path_;

    io_service &io_;
    net::unix::socket socket_;
    boost::beast::flat_buffer buffer_ { 8192 };
    beast::http::request<beast::http::empty_body> request_;
    beast::http::response<beast::http::string_body> response_;

    std::string host_;
    int version_;

    std::function<void(client<protocol_t> &)> on_connect_ =
        webvirt::http::on_connect<client<protocol_t>>;

    std::function<void(
        const beast::http::response<beast::http::string_body> &)>
        on_response_ = webvirt::http::on_response;

    std::function<void()> on_close_ = webvirt::http::on_close;

    std::function<void(const char *, beast::error_code)> on_error_ =
        webvirt::http::on_error;

public:
    explicit client(io_service &io, std::string socket_path,
                    std::string host = "localhost",
                    int version = webvirt::http::version::http_1_1)
        : socket_path_(std::move(socket_path))
        , io_(io)
        , socket_(boost::beast::net::make_strand(io_))
    {
        this->host(std::move(host)).version(version);
    }

    client &host(std::string value)
    {
        host_ = std::move(value);
        return *this;
    }

    const std::string &host() const
    {
        return host_;
    }

    client &version(int version)
    {
        version_ = version;
        return *this;
    }

    int version() const
    {
        return version_;
    }

    client &async_options(const char *target)
    {
        init_request(target);
        request_.method(beast::http::verb::options);
        async_connect();
        return *this;
    }

    client &async_get(const char *target)
    {
        init_request(target);
        request_.method(beast::http::verb::get);
        async_connect();
        return *this;
    }

    client &async_post(const char *target)
    {
        init_request(target);
        request_.method(beast::http::verb::post);
        async_connect();
        return *this;
    }

    void on_connect(std::function<void(client<protocol_t> &)> fn)
    {
        on_connect_ = fn;
    }

    void on_close(std::function<void()> fn)
    {
        on_close_ = fn;
    }

    void on_error(std::function<void(const char *, beast::error_code)> fn)
    {
        on_error_ = fn;
    }

    void
    on_response(std::function<
                void(const beast::http::response<beast::http::string_body> &)>
                    fn)
    {
        on_response_ = fn;
    }

    const beast::http::request<beast::http::empty_body> &request() const
    {
        return request_;
    }

    void close()
    {
        socket_.close();
    }

    std::size_t run()
    {
        return io_.process();
    }

private:
    void init_request(const char *target)
    {
        request_.version(version_);
        request_.target(target);
        request_.set(beast::http::field::host, host_);
        request_.set(beast::http::field::user_agent,
                     BOOST_BEAST_VERSION_STRING);
        request_.set(beast::http::field::accept, "*/*");
    }

    void async_connect()
    {
        socket_.async_connect(socket_path_,
                              boost::beast::bind_front_handler(
                                  &client<protocol_t>::client_async_on_connect,
                                  this->shared_from_this()));
    }

    void client_async_on_connect(boost::beast::error_code ec)
    {
        if (ec) {
            return on_error_(__func__, ec);
        }
        on_connect_(*this);

        beast::http::async_write(
            socket_,
            request_,
            boost::beast::bind_front_handler(
                &client<protocol_t>::client_async_on_write,
                this->shared_from_this()));
    }

    void client_async_on_write(boost::beast::error_code ec, std::size_t bytes)
    {
        boost::ignore_unused(bytes);
        if (ec) {
            return on_error_(__func__, ec);
        }

        beast::http::async_read(socket_,
                                buffer_,
                                response_,
                                boost::beast::bind_front_handler(
                                    &client<protocol_t>::client_async_on_read,
                                    this->shared_from_this()));
    }

    void client_async_on_read(boost::beast::error_code ec, std::size_t bytes)
    {
        boost::ignore_unused(bytes);
        if (ec) {
            return on_error_(__func__, ec);
        }
        on_response_(response_);

        socket_.shutdown(net::unix::socket::shutdown_both, ec);
        on_close_();
    }
};

using unix_client = client<net::unix>;

}; // namespace webvirt::http

#endif /* HTTP_CLIENT_HPP */
