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
    client(http::io_context &io, std::filesystem::path socket_path)
        : strand_(io)
        , socket_path_(std::move(socket_path))
        , ws_(std::make_shared<beast::websocket::stream<net::unix::socket>>(
              io))
    {
    }

    client &async_connect(const std::string &request_uri)
    {
        using namespace std::placeholders;
        beast::get_lowest_layer(*ws_).async_connect(
            socket_path_.c_str(),
            strand_.wrap(std::bind(&client::client_on_connect,
                                   shared_from_this(),
                                   _1,
                                   request_uri)));
        return *this;
    }

    client &async_write(const std::string &data)
    {
        using namespace std::placeholders;
        ws_->async_write(
            boost::asio::buffer(data),
            strand_.wrap(std::bind(
                &client::client_on_write, shared_from_this(), _1, _2)));
        return *this;
    }

    void close()
    {
        // Since a closure may happen at any point, async_close is called
        // within a boost::asio::post call to prioritize the closure
        // over other handlers that may have previously been called
        // (like async_read).
        using namespace std::placeholders;
        boost::asio::post(strand_.context(), [this] {
            ws_->async_close(beast::websocket::close_code::normal,
                             strand_.wrap(std::bind(&client::client_on_close,
                                                    shared_from_this(),
                                                    _1)));
        });
    }

    void shutdown(net::unix::socket::shutdown_type type)
    {
        beast::get_lowest_layer(*ws_).shutdown(type);
    }

    std::size_t run()
    {
        return strand_.context().run();
    }

    handler_setter(on_connect, on_connect_);
    handler_setter(on_handshake, on_handshake_);
    handler_setter(on_write, on_write_);
    handler_setter(on_read, on_read_);
    handler_setter(on_error, on_error_);

private:
    void client_on_connect(beast::error_code, const std::string &request_uri)
    {
        using namespace std::placeholders;

        ws_->set_option(beast::websocket::stream_base::timeout::suggested(
            beast::role_type::client));
        ws_->set_option(beast::websocket::stream_base::decorator(
            [](beast::websocket::request_type &req) { // LCOV_EXCL_LINE
                req.set(beast::http::field::user_agent,
                        std::string(BOOST_BEAST_VERSION_STRING) +
                            " websocket-client-async");
            }));
        ws_->text(true);

        on_connect_(shared_from_this());
        ws_->async_handshake(
            ws_response_,
            "localhost",
            request_uri,
            strand_.wrap(std::bind(
                &client::client_on_handshake, shared_from_this(), _1)));
    }

    void client_on_handshake(beast::error_code ec)
    {
        using namespace std::placeholders;

        if (ec) {
            logger::error(fmt::format("Client error: {}", ec.message()));
            return on_error_(ec.message().c_str(), ec);
        }

        on_handshake_(shared_from_this(), ws_response_);
        ws_->async_read(
            buffer_,
            strand_.wrap(std::bind(
                &client::client_on_read, shared_from_this(), _1, _2)));
    }

    void client_on_write(beast::error_code ec, std::size_t bytes)
    {
        boost::ignore_unused(bytes);

        logger::info(fmt::format("Sent {} bytes", bytes));

        if (ec) {
            logger::error(fmt::format("Client error: {}", ec.message()));
            return on_error_(ec.message().c_str(), ec);
        }

        on_write_(shared_from_this());
    }

    void client_on_read(beast::error_code ec, std::size_t bytes)
    {
        using namespace std::placeholders;
        boost::ignore_unused(bytes);

        if (ec) {
            logger::error(fmt::format("Client error: {}", ec.message()));
            return on_error_(ec.message().c_str(), ec);
        }

        on_read_(shared_from_this(), beast::buffers_to_string(buffer_.data()));
        buffer_.consume(bytes);

        ws_->async_read(
            buffer_,
            strand_.wrap(std::bind(
                &client::client_on_read, shared_from_this(), _1, _2)));
    }

    void client_on_close(beast::error_code)
    {
        return logger::info("Websocket client closed");
    }
};

}; // namespace webvirt::websocket

#endif /* WS_CLIENT_HPP */
