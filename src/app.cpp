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
#include "util/logging.hpp"
#include <app.hpp>
#include <http/middleware.hpp>
#include <virt/event.hpp>

#include <pugixml.hpp>

using namespace webvirt;

using http::middleware::with_libvirt;
using http::middleware::with_libvirt_domain;
using http::middleware::with_methods;

app::app(http::io_context &io, const std::filesystem::path &socket_path)
    : io_(io)
    , server_(io_, socket_path.string())
{
    router_.route(R"(^.+[^/]$)", bind(&app::append_trailing_slash));

    // Host-bound routes
    router_.route(R"(^/users/([^/]+)/host/)",
                  with_methods({ beast::http::verb::get },
                               with_libvirt(pool_,
                                            bind_libvirt(&views::host::show,
                                                         &host_view_))));
    router_.route(
        R"(^/users/([^/]+)/host/networks/)",
        with_methods(
            { beast::http::verb::get },
            with_libvirt(pool_,
                         bind_libvirt(&views::host::networks, &host_view_))));

    // Domain-bound routes
    router_.route(
        R"(^/users/([^/]+)/domains/$)",
        with_methods(
            { beast::http::verb::get },
            with_libvirt(
                pool_, bind_libvirt(&views::domains::index, &domains_view_))));
    router_.route(R"(^/users/([^/]+)/domains/([^/]+)/$)",
                  with_methods({ beast::http::verb::get },
                               with_libvirt_domain(
                                   pool_,
                                   bind_libvirt_domain(&views::domains::show,
                                                       &domains_view_))));
    router_.route(
        R"(^/users/([^/]+)/domains/([^/]+)/autostart/$)",
        with_methods(
            { beast::http::verb::post, beast::http::verb::delete_ },
            with_libvirt_domain(pool_,
                                bind_libvirt_domain(&views::domains::autostart,
                                                    &domains_view_))));
    router_.route(
        R"(^/users/([^/]+)/domains/([^/]+)/metadata/$)",
        with_methods(
            { beast::http::verb::post },
            with_libvirt_domain(pool_,
                                bind_libvirt_domain(&views::domains::metadata,
                                                    &domains_view_))));
    router_.route(
        R"(^/users/([^/]+)/domains/([^/]+)/bootmenu/$)",
        with_methods(
            { beast::http::verb::post, beast::http::verb::delete_ },
            with_libvirt_domain(pool_,
                                bind_libvirt_domain(&views::domains::bootmenu,
                                                    &domains_view_))));
    router_.route(R"(^/users/([^/]+)/domains/([^/]+)/start/$)",
                  with_methods({ beast::http::verb::post },
                               with_libvirt_domain(
                                   pool_,
                                   bind_libvirt_domain(&views::domains::start,
                                                       &domains_view_))));
    router_.route(
        R"(^/users/([^/]+)/domains/([^/]+)/shutdown/)",
        with_methods(
            { beast::http::verb::post },
            with_libvirt_domain(pool_,
                                bind_libvirt_domain(&views::domains::shutdown,
                                                    &domains_view_))));

    server_.on_request([this](http::connection_ptr http_conn,
                              const http::request &request,
                              http::response &response) {
        return router_.run(std::move(http_conn), request, response);
    });
}

app::~app()
{
    event_loop_ = false;
    if (event_thread_.joinable()) {
        event_thread_.join();
    }
}

std::size_t app::run()
{
    event_thread_ = std::thread(std::bind(&app::event_loop, this));

    std::unique_lock<std::mutex> guard(event_mutex_);
    event_cv_.wait(guard);

    if (event_error_) {
        logger::error("Event loop encountered an error during registration");
        return 0;
    }

    return server_.run();
}

virt::connection_pool &app::pool()
{
    return pool_;
}

void app::event_loop()
{
    if (virt::event::register_impl() == -1) {
        event_error_ = true;
    }
    logger::info("Registered default libvirt event implementation");

    logger::info("Event loop started");

    event_cv_.notify_one();
    if (event_error_) {
        return;
    }

    while (event_loop_) {
        // Run a single event loop iteration. Errors are logged
        // via libvirt's on error handler.
        virt::event::run_one();
    }

    logger::info("Event loop stopped");
}

void app::append_trailing_slash(http::connection_ptr,
                                const std::smatch &location,
                                const http::request &,
                                http::response &response)
{
    std::string uri(location[0]);
    uri.push_back('/');
    response.set(beast::http::field::location, uri);
    response.result(beast::http::status::temporary_redirect);
}
