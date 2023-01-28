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

    router.route(R"(/.+[^/]$)",
                 [](const auto &m, const auto &, auto &response) {
                     std::string uri(m[0]);
                     response.set(beast::http::field::location, uri + "/");
                     response.result(beast::http::status::moved_permanently);
                 });

    auto domains = http::router::with_user(
        [](const auto &user,
           const auto &,
           const beast::http::request<beast::http::dynamic_body> &,
           beast::http::response<beast::http::string_body> &response) {
            // Any response we send is JSON-serialized
            response.set("Content-Type", "application/json");

            Json::Value json(Json::arrayValue);
            virt::connection conn;
            try {
                conn.connect(virt::uri(user));
            } catch (const std::runtime_error &e) {
                std::cerr << e.what();
                json = Json::Value(Json::objectValue);
                json["detail"] = "Unable to connect to libvirt";
                response.result(beast::http::status::internal_server_error);
                auto output = json::stringify(json);
                response.body().append(output);
                response.content_length(response.body().size());
                return;
            }

            auto domains_ = conn.domains();
            for (auto &domain : domains_) {
                Json::Value map(Json::objectValue);
                for (auto &kv : domain) {
                    map[kv.first] = kv.second;
                }
                json.append(map);
            }

            auto output = json::stringify(json);
            response.body().append(output);
            response.content_length(response.body().size());
        });
    router.route(
        R"(/domains/)",
        http::router::with_methods({ beast::http::verb::post }, domains));

    auto domain = http::router::with_user(
        [](const auto &user, const auto &m, const auto &, auto &response) {
            const auto name = m[1];

            // Any response we send is JSON-serialized
            response.set("Content-Type", "application/json");

            Json::Value json(Json::objectValue);
            virt::connection conn;
            try {
                conn.connect(virt::uri(user));
            } catch (const std::runtime_error &e) {
                std::cerr << e.what();
                json = Json::Value(Json::objectValue);
                json["detail"] = "Unable to connect to libvirt";
                response.result(beast::http::status::internal_server_error);
                auto output = json::stringify(json);
                response.body().append(output);
                response.content_length(response.body().size());
                return;
            }

            auto domain = conn.domain(name);
            if (domain.find("id") == domain.end()) {
                json["detail"] = "Unable to find domain";
                response.result(beast::http::status::not_found);
            }

            for (auto &kv : domain) {
                json[kv.first] = kv.second;
            }

            auto output = json::stringify(json);
            response.body().append(output);
            response.content_length(response.body().size());
        });
    router.route(
        R"(/domains/([^/]+)/)",
        http::router::with_methods({ beast::http::verb::post }, domain));

    router.run();
}

void on_response(
    const boost::beast::http::response<boost::beast::http::string_body> &);

}; // namespace webvirt::http

#endif /* HTTP_HANDLERS_HPP */
