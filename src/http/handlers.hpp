/* Copyright (C) 2023 Kevin Morris <kevr@0cost.org> */
#ifndef HTTP_HANDLERS_HPP
#define HTTP_HANDLERS_HPP

#include "../json.hpp"
#include "../util.hpp"
#include "../virt/connection.hpp"
#include "../virt/util.hpp"
#include "router.hpp"
#include <boost/asio/buffer.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http/dynamic_body.hpp>
#include <libvirt/libvirt.h>
#include <sys/wait.h>
#include <thread>

namespace webvirt::http
{

template <typename client_t>
void on_connect(client_t &)
{
}

template <typename connection_t>
void on_accept(connection_t &)
{
}

void on_close();

void on_error(const char *, boost::beast::error_code);

template <typename connection_t>
void on_request(
    connection_t &,
    const boost::beast::http::request<boost::beast::http::dynamic_body> &,
    boost::beast::http::response<boost::beast::http::string_body> &)
{
}

void on_response(
    const boost::beast::http::response<boost::beast::http::string_body> &);

}; // namespace webvirt::http

#endif /* HTTP_HANDLERS_HPP */
