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
#ifndef WS_CONNECTION_HPP
#define WS_CONNECTION_HPP

#include <http/handlers.hpp>
#include <http/io_context.hpp>
#include <http/types.hpp>

#include <boost/beast/websocket.hpp>
#include <memory>

namespace webvirt::websocket
{

/** A websocket connection
 *
 * This class derives itself from an external socket that has already
 * established a connection with an endpoint, as well as an http::request
 * retrieved using boost::beast::http.
 *
 * The passed http::request should contain client websocket handshake data.
 **/
class connection : public std::enable_shared_from_this<connection>
{
private:
    http::io_context::strand strand_;
    beast::websocket::stream<net::unix::socket> ws_;
    beast::flat_buffer buffer_;

    http::request request_;

    http::handler<std::shared_ptr<connection>> on_accept_;
    http::handler<std::shared_ptr<connection>> on_handshake_;
    http::handler<std::shared_ptr<connection>, const std::string &> on_read_;
    http::handler<const char *, beast::error_code> on_error_;
    http::handler<> on_close_;

public:
    /** Construct a connection
     *
     * @param io webvirt::http::io_context reference
     * @param sock webvirt::net::unix::socket rvalue
     * @param request webvirt::http::request
     **/
    explicit connection(boost::asio::io_context &, net::unix::socket &&,
                        http::request request);

    /** Run the connection
     *
     * Dispatches a call to async_run to the io_context.
     **/
    void run();

    /** Write a text string to the websocket
     *
     * Calls beast::websocket::async_write using private
     * async_write as a handler.
     **/
    void write(const std::string &);

    /** Shutdown the underlying socket
     *
     * @param type webvirt::net::unix::socket::shutdown_type
     **/
    void shutdown(net::unix::socket::shutdown_type);

    handler_setter(on_accept, on_accept_);
    handler_setter(on_handshake, on_handshake_);
    handler_setter(on_read, on_read_);
    handler_setter(on_error, on_error_);
    handler_setter(on_close, on_close_);

private:
    void async_run();
    void async_accept(beast::error_code);
    void async_read(beast::error_code, std::size_t);
    void async_write(beast::error_code, std::size_t);
};

using connection_ptr = std::shared_ptr<connection>;

}; // namespace webvirt::websocket

#endif /* WS_CONNECTION_HPP */
