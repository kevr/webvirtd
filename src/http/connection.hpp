/* Copyright (C) 2023 Kevin Morris <kevr@0cost.org> */
#ifndef HTTP_CONNECTION_HPP
#define HTTP_CONNECTION_HPP

#include "handlers.hpp"
#include "io_service.hpp"
#include "namespaces.hpp"
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <iostream>
#include <memory>

namespace webvirt::http
{

template <typename acceptor_t, typename socket_t>
class connection
    : public std::enable_shared_from_this<connection<acceptor_t, socket_t>>
{
    socket_t socket_;
    boost::beast::flat_buffer buffer_ { 8192 };

    beast::http::request<beast::http::dynamic_body> request_;
    beast::http::response<beast::http::dynamic_body> response_;

    boost::asio::steady_timer deadline_;

    std::function<void(connection<net::unix::acceptor, net::unix::socket> &,
                       const beast::http::request<beast::http::dynamic_body> &,
                       beast::http::response<beast::http::dynamic_body> &)>
        on_request_ = webvirt::http::on_request<
            connection<net::unix::acceptor, net::unix::socket>>;

    std::function<void()> on_close_ = webvirt::http::on_close;

    std::function<void(const char *, beast::error_code)> on_error_ =
        webvirt::http::on_error;

public:
    explicit connection(socket_t socket, std::chrono::milliseconds ms)
        : socket_(std::move(socket))
        , deadline_(socket_.get_executor(), ms)
    {
    }

    void start()
    {
        read_request();
        check_deadline();
    }

    void on_error(std::function<void(const char *, beast::error_code)> fn)
    {
        on_error_ = fn;
    }

    void
    on_request(std::function<
               void(connection<net::unix::acceptor, net::unix::socket> &,
                    const beast::http::request<beast::http::dynamic_body> &,
                    beast::http::response<beast::http::dynamic_body> &)>
                   fn)
    {
        on_request_ = fn;
    }

    void on_close(std::function<void()> fn)
    {
        on_close_ = fn;
    }

    void close()
    {
        socket_.close();
    }

private:
    void read_request()
    {
        auto self = this->shared_from_this();

        const std::string func = __func__;
        beast::http::async_read(
            socket_,
            buffer_,
            request_,
            [self, func](beast::error_code ec, std::size_t bytes) {
                boost::ignore_unused(bytes);

                if (ec) {
                    return self->on_error_(func.c_str(), ec);
                }

                self->process_request();
            });
    }

    void process_request()
    {
        response_.version(request_.version());
        response_.keep_alive(false);
        response_.set(beast::http::field::content_type, "text/plain");
        response_.set(beast::http::field::server, BOOST_BEAST_VERSION_STRING);

        switch (request_.method()) {
        case beast::http::verb::get:
            response_.result(beast::http::status::ok);
            break;
        case beast::http::verb::post:
            response_.result(beast::http::status::ok);
            break;
        default:
            response_.result(beast::http::status::method_not_allowed);
            break;
        }

        create_response();
        write_response();
    }

    void create_response()
    {
        on_request_(*this, request_, response_);
    }

    void write_response()
    {
        auto self = this->shared_from_this();

        response_.content_length(response_.body().size());

        const std::string func = __func__;
        beast::http::async_write(
            socket_,
            response_,
            [self, func](boost::beast::error_code ec, std::size_t) {
                if (ec) {
                    return self->on_error_(func.c_str(), ec);
                }

                self->socket_.shutdown(net::unix::socket::shutdown_send, ec);
                self->deadline_.cancel();
                self->on_close_();
            });
    }

    void check_deadline()
    {
        auto self = this->shared_from_this();

        const std::string func = __func__;
        deadline_.async_wait([self, func](boost::beast::error_code ec) {
            self->socket_.close(ec);
            self->on_close_();
        });
    }
};

}; // namespace webvirt::http

#endif /* HTTP_CONNECTION_HPP */
