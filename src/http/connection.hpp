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

#include <boost/beast/core/bind_handler.hpp>
#include <boost/core/ignore_unused.hpp>
#include <http/handlers.hpp>
#include <http/io_context.hpp>
#include <http/types.hpp>
#include <util/logging.hpp>
#include <ws/connection.hpp>

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/core/detail/base64.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/compute/detail/sha1.hpp>
#include <iostream>
#include <memory>
#include <regex>

namespace webvirt::http
{

class connection : public std::enable_shared_from_this<connection>
{
    http::io_context::strand strand_;

    net::unix::socket socket_;
    std::shared_ptr<websocket::connection> websock_;

    boost::beast::flat_buffer buffer_ { 8192 };

    beast::http::request<beast::http::dynamic_body> request_;
    beast::http::response<beast::http::string_body> response_;

    boost::asio::steady_timer deadline_;

    handler<std::shared_ptr<connection>> on_accept_;
    handler<std::shared_ptr<connection>, const http::request &,
            http::response &>
        on_request_;
    handler<websocket::connection_ptr> on_websock_accept_;
    handler<websocket::connection_ptr> on_handshake_;
    handler<websocket::connection_ptr, const std::string &> on_websock_read_;
    handler<const char *, beast::error_code> on_error_;
    handler<> on_close_;

    bool upgrade_ { false };

public:
    typedef std::function<void(std::shared_ptr<http::connection>,
                               const std::smatch &, const http::request &,
                               http::response &)>
        route_function;

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

    std::shared_ptr<websocket::connection> websock() const
    {
        return websock_;
    }

    std::shared_ptr<websocket::connection> upgrade()
    {
        upgrade_ = true;

        // give socket_ to a newly created websocket::connection
        websock_ = std::make_shared<websocket::connection>(
            strand_.context(), std::move(socket_), request_);
        websock_->on_accept(on_websock_accept_);
        websock_->on_handshake(on_handshake_);
        websock_->on_read(on_websock_read_);
        websock_->on_error(on_error_);
        websock_->on_close(on_close_);

        return websock();
    }

    handler_setter(on_accept, on_accept_);
    handler_setter(on_request, on_request_);
    handler_setter(on_websock_accept, on_websock_accept_);
    handler_setter(on_handshake, on_handshake_);
    handler_setter(on_websock_read, on_websock_read_);
    handler_setter(on_error, on_error_);
    handler_setter(on_close, on_close_);

private:
    void read_request()
    {
        beast::http::async_read(
            socket_,
            buffer_,
            request_,
            strand_.wrap(std::bind(&connection::async_read,
                                   shared_from_this(),
                                   std::placeholders::_1,
                                   std::placeholders::_2)));
    }

    void async_read(beast::error_code ec, std::size_t bytes)
    {
        boost::ignore_unused(bytes);

        if (ec) {
            const std::string func = __func__;
            return on_error_(func.c_str(), ec);
        }

        process_request();
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

        on_request_(shared_from_this(), request_, response_);
        response_.content_length(response_.body().size());

        if (beast::websocket::is_upgrade(request_)) {
            logger::debug("Running websocket");
            deadline_.cancel();
            websock_->run();
        } else {
            beast::http::async_write(
                socket_,
                response_,
                strand_.wrap(std::bind(&connection::async_write,
                                       shared_from_this(),
                                       std::placeholders::_1,
                                       std::placeholders::_2)));
        }
    }

    void async_write(beast::error_code ec, std::size_t)
    {
        if (ec) {
            const std::string func = __func__;
            return on_error_(func.c_str(), ec);
        }

        deadline_.cancel();
        logger::debug("Closing socket");
        socket_.shutdown(net::unix::socket::shutdown_send, ec);
        on_close_();
    }

    void check_deadline()
    {
        deadline_.async_wait(
            strand_.wrap(std::bind(&connection::async_deadline,
                                   shared_from_this(),
                                   std::placeholders::_1)));
    }

    void async_deadline(beast::error_code ec)
    {
        if (ec == boost::asio::error::operation_aborted) {
            return;
        }

        socket_.close(ec);
        on_close_();
    }
};

using connection_ptr = std::shared_ptr<connection>;

}; // namespace webvirt::http

#endif /* HTTP_CONNECTION_HPP */
