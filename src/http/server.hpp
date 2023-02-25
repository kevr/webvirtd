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
#ifndef HTTP_SERVER_HPP
#define HTTP_SERVER_HPP

#include <http/connection.hpp>
#include <http/handlers.hpp>
#include <http/io_context.hpp>
#include <thread/worker_pool.hpp>
#include <util/config.hpp>
#include <util/logging.hpp>

#include <boost/asio.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <boost/beast.hpp>
#include <chrono>
#include <filesystem>
#include <functional>

namespace webvirt::net
{
using unix = boost::asio::local::stream_protocol;
};

namespace webvirt::http
{

/* An HTTP server */
class server
{
private:
    std::filesystem::path socket_path_;

    bool io_owned_ = true;
    io_context *io_ = nullptr;
    net::unix::stream_protocol::acceptor acceptor_;
    net::unix::socket socket_;

    std::chrono::milliseconds timeout_ = std::chrono::milliseconds(60 * 1000);

    handler<http::connection_ptr> on_accept_;
    handler<http::connection_ptr, const http::request &, http::response &>
        on_request_;
    handler<websocket::connection_ptr> on_websock_accept_;
    handler<websocket::connection_ptr> on_handshake_;
    handler<websocket::connection_ptr, const std::string &> on_websock_read_;
    handler<const char *, beast::error_code> on_error_;
    handler<> on_close_;

    thread::worker_pool pool_;

public:
    /** Construct a server without a webvirt::http::io_context
     *
     * @param socket_path Path to unix socket
     **/
    server(std::filesystem::path socket_path);

    /** Construct a server with an external webvirt::http::io_context
     *
     * @param io External webvirt::http::io_context
     * @param socket_path Path to unix socket
     **/
    server(io_context &io, std::filesystem::path socket_path);

    /** Destruct a server */
    ~server();

    /** Set connection timeout
     *
     * @param ms Timeout in milliseconds
     * @returns Reference to this
     **/
    server &timeout(std::chrono::milliseconds ms);

    /** Return connection timeout
     *
     * Returns connection timeout
     **/
    std::chrono::milliseconds timeout() const;

    /** Run the server's io_context
     *
     * @returns Number of handlers processed
     **/
    std::size_t run();

    handler_setter(on_accept, on_accept_);
    handler_setter(on_request, on_request_);
    handler_setter(on_websock_accept, on_websock_accept_);
    handler_setter(on_handshake, on_handshake_);
    handler_setter(on_websock_read, on_websock_read_);
    handler_setter(on_error, on_error_);
    handler_setter(on_close, on_close_);

private:
    void async_accept();
};

}; // namespace webvirt::http

#endif /* HTTP_SERVER_HPP */
