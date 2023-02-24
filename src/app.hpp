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

#include <regex>

namespace webvirt
{

class app
{
private:
    http::router router_;

    http::io_context &io_;
    http::server server_;

    views::host host_view_;
    views::domains domains_view_;

    virt::connection_pool pool_;

    std::atomic<bool> event_loop_ { true };
    std::atomic<bool> event_error_ { false };
    std::condition_variable event_cv_;
    std::mutex event_mutex_;
    std::thread event_thread_;

public:
    app(http::io_context &io, const std::filesystem::path &socket_path);
    ~app();

    std::size_t run();
    virt::connection_pool &pool();

private:
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

private:
    void event_loop();

private:
    void append_trailing_slash(http::connection_ptr, const std::smatch &,
                               const http::request &, http::response &);
};

}; // namespace webvirt

#endif /* APP_HPP */
