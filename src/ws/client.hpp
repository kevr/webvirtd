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
#ifndef WS_CLIENT_HPP
#define WS_CLIENT_HPP

#include <http/client.hpp>
#include <http/handlers.hpp>
#include <http/io_context.hpp>
#include <http/types.hpp>

#include <boost/beast/websocket.hpp>
#include <filesystem>
#include <functional>

namespace webvirt::websocket
{

/** A websocket client used for testing purposes */
class client : public std::enable_shared_from_this<client>
{
    http::io_context::strand strand_;
    std::filesystem::path socket_path_;

    std::shared_ptr<beast::websocket::stream<net::unix::socket>> ws_;
    beast::flat_buffer buffer_;
    beast::websocket::response_type ws_response_;

    http::handler<std::shared_ptr<client>> on_connect_;
    http::handler<std::shared_ptr<client>, beast::websocket::response_type>
        on_handshake_;
    http::handler<std::shared_ptr<client>> on_write_;
    http::handler<std::shared_ptr<client>, const std::string &> on_read_;
    http::handler<const char *, beast::error_code> on_error_;

public:
    /** Construct a client
     *
     * @param io webvirt::http::io_context
     * @param socket_path Path to a unix socket
     **/
    client(http::io_context &, std::filesystem::path);

    /** Begin async connection to `request_uri`
     *
     * @param request_uri HTTP request URI to websocket endpoint
     * @returns Reference to this client
     **/
    client &async_connect(const std::string &);

    /** Begin an async write of `data`
     *
     * @param data Text data to write to the websocket
     * @returns Reference to this client
     **/
    client &async_write(const std::string &);

    /** Close the client websocket */
    void close();

    /** Shutdown the underlying socket
     *
     * @param type webvirt::net::unix::socket::shutdown_type
     **/
    void shutdown(net::unix::socket::shutdown_type);

    /** Run the client strand's io_context
     *
     * @return Number of handlers processed
     **/
    std::size_t run();

    handler_setter(on_connect, on_connect_);
    handler_setter(on_handshake, on_handshake_);
    handler_setter(on_write, on_write_);
    handler_setter(on_read, on_read_);
    handler_setter(on_error, on_error_);

private:
    void client_on_connect(beast::error_code, const std::string &);
    void client_on_handshake(beast::error_code);
    void client_on_write(beast::error_code, std::size_t);
    void client_on_read(beast::error_code, std::size_t);
    void client_on_close(beast::error_code);
};

}; // namespace webvirt::websocket

#endif /* WS_CLIENT_HPP */
