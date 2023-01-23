/* Copyright (C) 2023 Kevin Morris <kevr@0cost.org> */
#include "handlers.hpp"
#include <iostream>

void webvirt::http::on_close()
{
}

void webvirt::http::on_error(const char *source, boost::beast::error_code ec)
{
    std::string message(source);
    message.append(": ");
    message.append(ec.message());
    message.push_back('\n');
    std::cerr << message;
}

void webvirt::http::on_response(
    const boost::beast::http::response<boost::beast::http::string_body> &)
{
}
