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
#include "app.hpp"
#include "http/middleware.hpp"
#include <pugixml.hpp>
using namespace webvirt;

using http::middleware::with_libvirt;
using http::middleware::with_methods;

app::app(webvirt::io_service &io, const std::filesystem::path &socket_path)
    : io_(io)
    , server_(io_, socket_path.string())
{
    router_.route(R"(^.+[^/]$)", bind(&app::append_trailing_slash));
    router_.route(R"(^/users/([^/]+)/domains/$)",
                  with_methods({ beast::http::verb::get },
                               with_libvirt(bind_libvirt(
                                   &views::domains::index, &domains_view_))));
    router_.route(R"(^/users/([^/]+)/domains/([^/]+)/$)",
                  with_methods({ beast::http::verb::get },
                               with_libvirt(bind_libvirt(&views::domains::show,
                                                         &domains_view_))));
    router_.route(
        R"(^/users/([^/]+)/domains/([^/]+)/autostart/$)",
        with_methods({ beast::http::verb::post, beast::http::verb::delete_ },
                     with_libvirt(bind_libvirt(&views::domains::autostart,
                                               &domains_view_))));
    router_.route(
        R"(^/users/([^/]+)/domains/([^/]+)/metadata/$)",
        with_methods({ beast::http::verb::post },
                     with_libvirt(bind_libvirt(&views::domains::metadata,
                                               &domains_view_))));
    router_.route(R"(^/users/([^/]+)/domains/([^/]+)/start/$)",
                  with_methods({ beast::http::verb::post },
                               with_libvirt(bind_libvirt(
                                   &views::domains::start, &domains_view_))));
    router_.route(
        R"(^/users/([^/]+)/domains/([^/]+)/shutdown/)",
        with_methods({ beast::http::verb::post },
                     with_libvirt(bind_libvirt(&views::domains::shutdown,
                                               &domains_view_))));

    server_.on_request([this](auto &, const auto &request, auto &response) {
        return router_.run(request, response);
    });
}

std::size_t app::run()
{
    return server_.run();
}

void app::append_trailing_slash(const std::smatch &location,
                                const http::request &,
                                http::response &response)
{
    std::string uri(location[0]);
    uri.push_back('/');
    response.set(beast::http::field::location, uri);
    response.result(beast::http::status::temporary_redirect);
}
