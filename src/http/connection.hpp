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
#ifndef HTTP_CONNECTION_HPP
#define HTTP_CONNECTION_HPP

#include <http/handlers.hpp>
#include <http/io_context.hpp>
#include <http/types.hpp>
#include <util/logging.hpp>

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <iostream>
#include <memory>

namespace webvirt::http
{

class connection : public std::enable_shared_from_this<connection>
{
    http::io_context::strand strand_;

    net::unix::socket socket_;
    boost::beast::flat_buffer buffer_ { 8192 };

    beast::http::request<beast::http::dynamic_body> request_;
    beast::http::response<beast::http::string_body> response_;

    boost::asio::steady_timer deadline_;

    handler<connection &, const http::request &, http::response &> on_request_;
    handler<const char *, beast::error_code> on_error_;
    handler<> on_close_;

public:
    explicit connection(http::io_context &io, net::unix::socket socket,
                        std::chrono::milliseconds ms)
        : strand_(io)
        , socket_(std::move(socket))
        , deadline_(socket_.get_executor(), ms)
    {
    }

    void start()
    {
        read_request();
        check_deadline();
    }

    void close()
    {
        socket_.close();
    }

    handler_setter(on_request, on_request_);
    handler_setter(on_error, on_error_);
    handler_setter(on_close, on_close_);

private:
    void read_request()
    {
        auto self = this->shared_from_this();

        const std::string func = __func__;
        beast::http::async_read(socket_,
                                buffer_,
                                request_,
                                strand_.wrap([self, func](beast::error_code ec,
                                                          std::size_t bytes) {
                                    boost::ignore_unused(bytes);

                                    if (ec) {
                                        return self->on_error_(func.c_str(),
                                                               ec);
                                    }

                                    self->process_request();
                                }));
    }

    void process_request()
    {
        logger::debug([this] {
            const std::string method =
                std::string(request_.method_string().data(),
                            request_.method_string().size());
            const std::string target = std::string(request_.target().data(),
                                                   request_.target().size());
            return fmt::format(
                "Received \"{} {} HTTP/{}.{}\"",
                method,
                target,
                boost::numeric_cast<unsigned int>(request_.version() / 10),
                boost::numeric_cast<unsigned int>(request_.version() % 10));
        });

        response_.version(request_.version());
        response_.keep_alive(false);
        response_.set(beast::http::field::content_type, "text/plain");
        response_.set(beast::http::field::server, BOOST_BEAST_VERSION_STRING);

        create_response();
        write_response();
    }

    void create_response()
    {
        on_request_(*this, request_, response_);
    }

    void write_response()
    {
        auto self = this->shared_from_this();

        response_.content_length(response_.body().size());

        const std::string func = __func__;
        beast::http::async_write(
            socket_,
            response_,
            strand_.wrap([self, func](boost::beast::error_code ec,
                                      std::size_t) {
                if (ec) {
                    return self->on_error_(func.c_str(), ec);
                }

                self->socket_.shutdown(net::unix::socket::shutdown_send, ec);
                self->deadline_.cancel();
                self->on_close_();
            }));
    }

    void check_deadline()
    {
        auto self = this->shared_from_this();

        const std::string func = __func__;
        deadline_.async_wait(
            strand_.wrap([self, func](boost::beast::error_code ec) {
                self->socket_.close(ec);
                self->on_close_();
            }));
    }
};

}; // namespace webvirt::http

#endif /* HTTP_CONNECTION_HPP */
