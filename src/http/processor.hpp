/* Copyright (C) 2023 Kevin Morris <kevr@0cost.org> */
#ifndef HTTP_PROCESSOR_HPP
#define HTTP_PROCESSOR_HPP

#include "namespaces.hpp"
#include <boost/beast.hpp>
#include <map>
#include <vector>

namespace webvirt::http
{

bool allowed_methods(const std::vector<boost::beast::http::verb> &methods,
                     boost::beast::http::verb method);

class processor
{
private:
    using request_t = beast::http::request<beast::http::dynamic_body>;
    const request_t &request_;

    using response_t = beast::http::response<beast::http::dynamic_body>;
    response_t &response_;

    using route_function_t =
        std::function<void(const request_t &, response_t &)>;
    std::map<std::string, route_function_t> routes_;

public:
    explicit processor(const request_t &request, response_t &response);
    void run();
    void route(std::string request_uri,
               std::function<void(const request_t &, response_t &)> fn);
};

}; // namespace webvirt::http

#endif /* HTTP_PROCESSOR_HPP */
