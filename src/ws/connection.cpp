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
#include <boost/beast/core/stream_traits.hpp>
#include <util/config.hpp>
#include <util/logging.hpp>
#include <ws/connection.hpp>

#include <boost/asio/placeholders.hpp>
#include <iostream>

using namespace webvirt::websocket;

using std::placeholders::_1;
using std::placeholders::_2;

connection::connection(boost::asio::io_context &io, net::unix::socket &&sock,
                       http::request request)
    : strand_(io)
    , ws_(std::move(sock))
    , request_(std::move(request))
{
}

void connection::run()
{
    logger::debug("Dispatching websocket connection");
    boost::asio::dispatch(
        ws_.get_executor(),
        strand_.wrap(std::bind(&connection::async_run, shared_from_this())));
}

void connection::write(const std::string &str)
{
    ws_.async_write(
        boost::asio::buffer(str),
        strand_.wrap(
            std::bind(&connection::async_write, shared_from_this(), _1, _2)));
}

void connection::shutdown(net::unix::socket::shutdown_type type)
{
    beast::get_lowest_layer(ws_).shutdown(type);
}

void connection::async_run()
{
    // Set suggested timeout settings for the websocket
    ws_.set_option(beast::websocket::stream_base::timeout::suggested(
        beast::role_type::server));

    // Set a decorator to change the Server of the handshake
    ws_.set_option(beast::websocket::stream_base::decorator(
        [](beast::websocket::response_type &res) { // LCOV_EXCL_LINE
            res.set(beast::http::field::server,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                        " websocket-server-async");
        }));

    ws_.text(true);

    on_accept_(shared_from_this());

    logger::debug("Waiting for websocket client handshake");
    // Accept the websocket handshake
    ws_.async_accept(request_,
                     strand_.wrap(std::bind(
                         &connection::async_accept, shared_from_this(), _1)));
}

void connection::async_accept(beast::error_code ec)
{
    logger::debug("Websocket handshake exchanged");

    if (ec) {
        logger::error(ec.message());
        return on_error_(ec.message().c_str(), ec);
    }

    // check_heartbeat();
    logger::info("Waiting for data...");
    on_handshake_(shared_from_this());
    ws_.async_read(buffer_,
                   std::bind(&connection::async_read,
                             shared_from_this(),
                             std::placeholders::_1,
                             std::placeholders::_2));
}

void connection::async_read(beast::error_code ec, std::size_t bytes)
{
    boost::ignore_unused(bytes);

    if (ec == beast::websocket::error::closed) {
        logger::debug("Websocket closed");
        return on_close_();
    } else if (ec) {
        logger::error(ec.message());
        return on_error_(ec.message().c_str(), ec);
    }

    logger::info(
        fmt::format("WS Read: {}", beast::buffers_to_string(buffer_.data())));
    buffer_.consume(bytes);

    logger::info("Waiting for data...");
    ws_.async_read(buffer_,
                   strand_.wrap(std::bind(
                       &connection::async_read, shared_from_this(), _1, _2)));
}

void connection::async_write(beast::error_code ec, std::size_t bytes)
{
    boost::ignore_unused(bytes);

    if (ec) {
        logger::error(ec.message());
        return on_error_(ec.message().c_str(), ec);
    }
}
