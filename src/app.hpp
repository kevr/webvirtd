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
#ifndef APP_HPP
#define APP_HPP

#include <http/io_context.hpp>
#include <http/router.hpp>
#include <http/server.hpp>
#include <views/domains.hpp>
#include <views/host.hpp>
#include <virt/connection_pool.hpp>
#include <virt/events.hpp>
#include <virt/events/lifecycle.hpp>
#include <ws/connection.hpp>
#include <ws/pool.hpp>

#include <atomic>
#include <map>
#include <regex>
#include <vector>

namespace webvirt
{

/** A class representing the webvirtd application
 *
 * This class can be used to run the webvirtd application's main loop,
 * which, when run():
 *
 * 1. Configures internal http::router
 * 2. Handles a libvirt event thread
 * 3. Runs internal http::server, passing HTTP requests to http::router::run
 **/
class app
{
private:
    http::router router_;

    http::io_context &io_;
    http::server server_;

    views::host host_view_;
    views::domains domains_view_;

    virt::connection_pool pool_;

    websocket::pool websockets_;

    // username -> virt::events
    std::map<std::string, virt::events> events_;

    std::atomic<bool> event_loop_ { true };
    std::atomic<bool> event_error_ { false };
    std::condition_variable event_cv_;
    std::mutex event_mutex_;
    std::thread event_thread_;

    http::handler<virt::connection &> on_virt_event_registration_;

public:
    /** Construct the application
     *
     * @param io webvirt::http::io_context
     * @param socket_path Path to unix socket
     **/
    app(http::io_context &io, const std::filesystem::path &socket_path);

    /** Destruct the application */
    ~app();

    /** Run the application
     *
     * 1. Start a threaded libvirt event loop
     * 2. Run the internal http::server
     *
     * @returns Number of handlers processed by http::server's io_context
     **/
    std::size_t run();

    /** Returns a reference to internal libvirt connection pool
     *
     * @returns Reference to internal libvirt connection pool
     **/
    virt::connection_pool &pool();

    /** Returns a reference to internal http::server
     *
     * This reference is primarily used when testing internal implementations
     * of this class externally.
     *
     * @returns Reference to internal http::server
     **/
    http::server &server();

    /** Remove application events for a particular virt::connection_pool
     *
     * @param conn libvirt connection
     **/
    void remove_events(virt::connection &);

    /** Add application events for a particular virt::connection
     *
     * @param conn libvirt connection
     * @param user libvirt connection's username
     * @param lifecycle_cb Lifecycle event callback
     **/
    void add_events(
        virt::connection &,
        const virt::lifecycle_callback &lifecycle_cb =
            virt::lifecycle_callback(virt::lifecycle_event::on_event_handler));

    /** Return events bound to username
     *
     * @param username libvirt user's username
     * @returns Reference to virt::events mapped to username
     **/
    virt::events &events(const std::string &);

    handler_setter(on_virt_event_registration, on_virt_event_registration_);

private: // Utilities
    template <typename Func, typename Pointer>
    auto bind(Func fn, Pointer ptr)
    {
        using namespace std::placeholders;
        return std::bind(fn, ptr, _1, _2, _3, _4);
    }

    template <typename Func, typename Pointer>
    auto bind_libvirt(Func fn, Pointer ptr)
    {
        using namespace std::placeholders;
        return std::bind(fn, ptr, _1, _2, _3, _4, _5);
    }

    template <typename Func, typename Pointer>
    auto bind_libvirt_domain(Func fn, Pointer ptr)
    {
        using namespace std::placeholders;
        return std::bind(fn, ptr, _1, _2, _3, _4, _5, _6);
    }

private: // Handlers
    void event_loop();

private: // Routes
    void append_trailing_slash(http::connection_ptr, const std::smatch &,
                               const http::request &, http::response &);
    void websocket(virt::connection &, http::connection_ptr,
                   const std::smatch &, const http::request &,
                   http::response &);
};

}; // namespace webvirt

#endif /* APP_HPP */
