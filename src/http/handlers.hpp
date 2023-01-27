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
    const boost::beast::http::request<boost::beast::http::dynamic_body>
        &request,
    boost::beast::http::response<boost::beast::http::string_body> &response)
{
    http::router router(request, response);

    router.route(
        "/domains",
        [](const beast::http::request<beast::http::dynamic_body> &request,
           beast::http::response<beast::http::string_body> &response) {
            if (!http::allowed_methods({ boost::beast::http::verb::post },
                                       request.method())) {
                return response.result(
                    beast::http::status::method_not_allowed);
            }

            // Any response we send is JSON-serialized
            response.set("Content-Type", "application/json");

            // Parse expected JSON from the request body.
            Json::Value root;
            try {
                root = json::parse(request.body());
            } catch (const std::invalid_argument &e) {
                std::cerr << e.what();
                Json::Value error;
                error["error"] = "unable to parse request body json";
                response.body().append(json::stringify(error));
                return response.result(beast::http::status::bad_request);
            }

            auto &sys = syscaller::instance();
            auto user = root.get("user", "").asString();
            auto *passwd = sys.getpwnam(user.c_str());
            if (!passwd) {
                std::cerr << "error: unable to find user\n";
                return response.result(beast::http::status::not_found);
            }

            Json::Value json(Json::arrayValue);
            virt::connection conn(virt::uri(passwd->pw_name));
            auto domains_ = conn.domains();
            for (auto &domain : domains_) {
                Json::Value map(Json::objectValue);
                for (auto &kv : domain) {
                    map[kv.first] = kv.second;
                }
                json.append(map);
            }

            Json::FastWriter writer;
            std::stringstream ss;
            ss << writer.write(json);
            auto output = ss.str();

            response.body().append(output);
            response.content_length(response.body().size());
        });

    router.run();
}

void on_response(
    const boost::beast::http::response<boost::beast::http::string_body> &);

}; // namespace webvirt::http

#endif /* HTTP_HANDLERS_HPP */
