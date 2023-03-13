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
#include <app.hpp>
#include <data/domain.hpp>
#include <http/middleware.hpp>
#include <util/logging.hpp>
#include <virt/events/callbacks/lifecycle.hpp>
#include <virt/events/lifecycle.hpp>

#include <pugixml.hpp>

using namespace webvirt;

using http::middleware::with_libvirt;
using http::middleware::with_libvirt_domain;
using http::middleware::with_methods;

app::app(http::io_context &io, const std::filesystem::path &socket_path)
    : io_(io)
    , server_(io_, socket_path.string())
{
    // General routes
    router_.route(R"(^.+[^/]$)", bind(&app::append_trailing_slash, this));

    // Websocket routes
    router_.route(
        R"(^/users/([^/]+)/websocket/)",
        with_methods(
            { beast::http::verb::get },
            with_libvirt(pool_, bind_libvirt(&app::websocket, this))));

    // Host routes
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

    // Domain routes
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

http::server &app::server()
{
    return server_;
}

static constexpr int TARGET_LIFECYCLE_EVENTS =
    (1 << VIR_DOMAIN_EVENT_STARTED) | (1 << VIR_DOMAIN_EVENT_SHUTDOWN) |
    (1 << VIR_DOMAIN_EVENT_STOPPED);

void app::remove_events(virt::connection &conn)
{
    std::lock_guard<std::mutex> guard(events_mutex_);

    const auto &user = conn.user();
    auto it = events_.find(user);
    if (it != events_.end()) {
        auto &events = it->second;
        events.remove(VIR_DOMAIN_EVENT_ID_LIFECYCLE);
        if (events.size() == 0) {
            events_.erase(it);
        }
    }
}

void app::add_events(virt::connection &conn,
                     const virt::lifecycle_callback &lifecycle_cb)
{
    std::lock_guard<std::mutex> guard(events_mutex_);

    const auto &user = conn.user();
    auto ev = std::make_shared<virt::lifecycle_event>(
        conn, lifecycle_cb, [this, user](auto &, auto &domain, int type, int) {
            if ((1 << type) & TARGET_LIFECYCLE_EVENTS) {
                websockets_.broadcast(user, data::simple_domain(domain));
            }
        });

    auto &user_events = events_[user];
    user_events.set(ev->id(), std::move(ev));

    on_virt_event_registration_(conn);
}

virt::events &app::events(const std::string &username)
{
    std::lock_guard<std::mutex> guard(events_mutex_);
    return events_.at(username);
}

void app::event_loop()
{
    if (virt::event::register_impl() == -1) {
        event_error_ = true;
    }
    logger::info("Registered default libvirt event implementation");

    event_cv_.notify_one();
    if (event_error_) {
        return;
    }

    logger::info("Event loop started");
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

void app::websocket(virt::connection &conn, http::connection_ptr http_conn,
                    const std::smatch &, const http::request &,
                    http::response &response)
{
    // Grab a local reference to libvirt connection's user, as it will
    // be reused multiple times throughout this function.
    const auto &user = conn.user();

    // Upgrade the HTTP connection into a Websocket connection
    websocket::connection_ptr ws_conn = http_conn->upgrade();

    // On successful websocket handshake, add libvirt events for the
    // connection if they don't yet exist.
    ws_conn->on_handshake([this, &conn, user](auto) {
        if (events_.find(conn.user()) == events_.end()) {
            add_events(conn);
        }
    });

    // On close, remove the connection from internal websockets_ map
    // bucket pertaining to `user`.
    ws_conn->on_close([this, &conn, user, ws_conn] {
        // On closure, remove the ptr from websockets_
        websockets_.remove(user, ws_conn);

        // If we just removed the last websocket from the user's pool
        if (!websockets_[user].size()) {
            // Then erase the user's event objects
            remove_events(conn);
        }
    });

    // Finally, add the new Websocket connection, `ws_conn`, to internal
    // websockets_ map under the `user` bucket.
    websockets_.add(user, std::move(ws_conn));

    // Setup response status for logging purposes.
    response.result(beast::http::status::switching_protocols);
}
