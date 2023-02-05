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

#include "http/router.hpp"
#include "http/server.hpp"
#include "views/domains.hpp"
#include <regex>

namespace webvirt
{

class app
{
private:
    http::router router_;

    webvirt::io_service &io_;
    http::server<net::unix> server_;

    views::domains domains_view_;

public:
    app(webvirt::io_service &io, const std::filesystem::path &socket_path);
    std::size_t run();

private:
    template <typename Func>
    auto bind(Func fn)
    {
        return std::bind(fn,
                         this,
                         std::placeholders::_1,
                         std::placeholders::_2,
                         std::placeholders::_3);
    }

    template <typename Func, typename Pointer>
    auto bind_libvirt(Func fn, Pointer ptr)
    {
        return std::bind(fn,
                         ptr,
                         std::placeholders::_1,
                         std::placeholders::_2,
                         std::placeholders::_3,
                         std::placeholders::_4,
                         std::placeholders::_5);
    }

private:
    void append_trailing_slash(const std::smatch &, const http::request &,
                               http::response &);
};

}; // namespace webvirt

#endif /* APP_HPP */
