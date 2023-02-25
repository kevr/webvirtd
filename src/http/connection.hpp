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
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <iostream>
#include <memory>
#include <regex>

namespace webvirt::http
{

/** HTTP server-side connection */
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
    /** HTTP route function signature */
    typedef std::function<void(std::shared_ptr<http::connection>,
                               const std::smatch &, const http::request &,
                               http::response &)>
        route_function;

public:
    /** Construct a connection
     *
     * @param io webvirt::http::io_context
     * @param socket webvirt::net::unix::socket
     * @param ms Connection timeout in milliseconds
     **/
    explicit connection(http::io_context &io, net::unix::socket socket,
                        std::chrono::milliseconds ms);

    /** Start the connection */
    void start();

    /** Close the connection */
    void close();

    /** Return internal websocket::connection_ptr
     *
     * The websocket::connection_ptr returned by this function is only
     * valid once upgrade() has been called.
     *
     * @returns Internal websocket::connection_ptr
     **/
    std::shared_ptr<websocket::connection> websock() const;

    /** Upgrade this connection to a websocket connection
     *
     * @returns Internal websocket::connection_ptr
     **/
    std::shared_ptr<websocket::connection> upgrade();

    handler_setter(on_accept, on_accept_);
    handler_setter(on_request, on_request_);
    handler_setter(on_websock_accept, on_websock_accept_);
    handler_setter(on_handshake, on_handshake_);
    handler_setter(on_websock_read, on_websock_read_);
    handler_setter(on_error, on_error_);
    handler_setter(on_close, on_close_);

private:
    void read_request();
    void process_request();
    void check_deadline();

    void async_read(beast::error_code, std::size_t);
    void async_write(beast::error_code, std::size_t);
    void async_deadline(beast::error_code);
};

using connection_ptr = std::shared_ptr<connection>;

}; // namespace webvirt::http

#endif /* HTTP_CONNECTION_HPP */
