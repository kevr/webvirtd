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
#include <http/connection.hpp>

using namespace webvirt;
using namespace http;

connection::connection(http::io_context &io, net::unix::socket socket,
                       std::chrono::milliseconds ms)
    : strand_(io)
    , socket_(std::move(socket))
    , deadline_(socket_.get_executor(), ms)
{
}

void connection::start()
{
    read_request();
    check_deadline();
}

void connection::close()
{
    socket_.close();
}

std::shared_ptr<websocket::connection> connection::websock() const
{
    return websock_;
}

std::shared_ptr<websocket::connection> connection::upgrade()
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

void connection::read_request()
{
    beast::http::async_read(socket_,
                            buffer_,
                            request_,
                            strand_.wrap(std::bind(&connection::async_read,
                                                   shared_from_this(),
                                                   std::placeholders::_1,
                                                   std::placeholders::_2)));
}

void connection::process_request()
{
    logger::debug([this] {
        const std::string method = std::string(
            request_.method_string().data(), request_.method_string().size());
        const std::string target =
            std::string(request_.target().data(), request_.target().size());
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

    CLASS_TRACE("Processed request");
    if (beast::websocket::is_upgrade(request_)) {
        CLASS_TRACE("Running websocket");
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

void connection::check_deadline()
{
    deadline_.async_wait(strand_.wrap(std::bind(&connection::async_deadline,
                                                shared_from_this(),
                                                std::placeholders::_1)));
}

void connection::async_read(beast::error_code ec, std::size_t bytes)
{
    boost::ignore_unused(bytes);

    if (ec) {
        CLASS_ETRACE(ec.message());
        const std::string func = __func__;
        return on_error_(func.c_str(), ec);
    }

    process_request();
}

void connection::async_write(beast::error_code ec, std::size_t)
{
    if (ec) {
        CLASS_ETRACE(ec.message());
        const std::string func = __func__;
        return on_error_(func.c_str(), ec);
    }

    deadline_.cancel();
    CLASS_TRACE("Closing socket");
    socket_.shutdown(net::unix::socket::shutdown_send, ec);
    on_close_();
}

void connection::async_deadline(beast::error_code ec)
{
    if (ec == boost::asio::error::operation_aborted) {
        CLASS_TRACE(ec.message());
        return;
    }

    socket_.close(ec);
    on_close_();
}
