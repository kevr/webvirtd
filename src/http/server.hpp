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
    server(std::filesystem::path socket_path)
        : socket_path_(std::move(socket_path))
        , io_(new io_context())
        , acceptor_(*io_, socket_path_.string())
        , socket_(*io_)
        , pool_(*io_)
    {
    }

    server(io_context &io, std::filesystem::path socket_path)
        : socket_path_(std::move(socket_path))
        , io_owned_(false)
        , io_(&io)
        , acceptor_(*io_, socket_path_.string())
        , socket_(*io_)
        , pool_(*io_)
    {
    }

    ~server()
    {
        pool_.join();

        if (io_owned_)
            delete io_;
    }

    server &timeout(std::chrono::milliseconds ms)
    {
        timeout_ = ms;
        return *this;
    }

    std::chrono::milliseconds timeout() const
    {
        return timeout_;
    }

    std::size_t run()
    {
        logger::info(fmt::format("Listening on '{}'", socket_path_.c_str()));

        // Begin async calls
        async_accept();

        // Start up internal worker_pool
        const auto threads = config::ref().get<unsigned>("threads");
        pool_.start(threads);

        // Run the io_context on the main thread
        return io_->run();
    }

    handler_setter(on_accept, on_accept_);
    handler_setter(on_request, on_request_);
    handler_setter(on_websock_accept, on_websock_accept_);
    handler_setter(on_handshake, on_handshake_);
    handler_setter(on_websock_read, on_websock_read_);
    handler_setter(on_error, on_error_);
    handler_setter(on_close, on_close_);

private:
    void async_accept()
    {
        acceptor_.async_accept(socket_, [this](boost::beast::error_code) {
            // Here, we don't handle beast::error_code; we let that
            // job fall through to the connection we make. If there's
            // a problem with the socket, operations will fails within
            // the connection immediately, which calls on_error_.
            auto conn = std::make_shared<connection>(
                *io_, std::move(socket_), timeout_);

            on_accept_(conn);
            conn->on_accept(on_accept_);
            conn->on_request(on_request_);
            conn->on_websock_accept(on_websock_accept_);
            conn->on_handshake(on_handshake_);
            conn->on_websock_read(on_websock_read_);
            conn->on_error(on_error_);
            conn->on_close(on_close_);
            conn->start();

            async_accept();
        });
    }
};

}; // namespace webvirt::http

#endif /* HTTP_SERVER_HPP */
