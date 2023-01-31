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

#include "connection.hpp"
#include "handlers.hpp"
#include "io_service.hpp"
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

template <typename protocol_t>
class server
{
private:
    std::filesystem::path socket_path_;

    bool io_owned_ = true;
    io_service *io_ = nullptr;
    typename protocol_t::acceptor acceptor_;
    typename protocol_t::socket socket_;

    std::chrono::milliseconds timeout_ = std::chrono::milliseconds(60 * 1000);

    std::function<void(connection<net::unix::acceptor, net::unix::socket> &)>
        on_accept_ = webvirt::http::on_accept<
            connection<net::unix::acceptor, net::unix::socket>>;

    std::function<void(
        connection<net::unix::acceptor, net::unix::socket> &,
        const boost::beast::http::request<boost::beast::http::dynamic_body> &,
        boost::beast::http::response<boost::beast::http::string_body> &)>
        on_request_ = webvirt::http::on_request<
            connection<net::unix::acceptor, net::unix::socket>>;

    std::function<void()> on_close_ = webvirt::http::on_close;

    std::function<void(const char *, beast::error_code)> on_error_ =
        webvirt::http::on_error;

public:
    server(std::filesystem::path socket_path)
        : socket_path_(std::move(socket_path))
        , io_(new io_service())
        , acceptor_(*io_, socket_path_.string())
        , socket_(*io_)
    {
    }

    server(io_service &io, std::filesystem::path socket_path)
        : socket_path_(std::move(socket_path))
        , io_owned_(false)
        , io_(&io)
        , acceptor_(*io_, socket_path_.string())
        , socket_(*io_)
    {
    }

    ~server()
    {
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
        async_accept();
        return io_->process();
    }

    void on_accept(std::function<
                   void(connection<net::unix::acceptor, net::unix::socket> &)>
                       fn)
    {
        on_accept_ = fn;
    }

    void on_request(
        std::function<void(
            connection<net::unix::acceptor, net::unix::socket> &,
            const boost::beast::http::request<boost::beast::http::dynamic_body>
                &,
            boost::beast::http::response<boost::beast::http::string_body> &)>
            fn)
    {
        on_request_ = fn;
    }

    void on_error(std::function<void(const char *, beast::error_code)> fn)
    {
        on_error_ = fn;
    }

    void on_close(std::function<void()> fn)
    {
        on_close_ = fn;
    }

private:
    void async_accept()
    {
        const auto func = __func__;
        acceptor_.async_accept(
            socket_, [this, &func](boost::beast::error_code) {
                // Here, we don't handle beast::error_code; we let that
                // job fall through to the connection we make. If there's
                // a problem with the socket, operations will fails within
                // the connection immediately, which calls on_error_.

                auto conn = std::make_shared<
                    connection<net::unix::acceptor, net::unix::socket>>(
                    std::move(socket_), timeout_);
                on_accept_(*conn);
                conn->on_error(on_error_);
                conn->on_close(on_close_);
                conn->on_request(on_request_);
                conn->start();

                async_accept();
            });
    }
};

using unix_server = server<net::unix>;

}; // namespace webvirt::http

#endif /* HTTP_SERVER_HPP */
