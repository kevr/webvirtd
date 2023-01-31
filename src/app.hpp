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
#include <regex>

namespace webvirt
{

class app
{
private:
    http::router router_;

    webvirt::io_service &io_;
    http::server<net::unix> server_;

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

    template <typename Func>
    auto bind_user(Func fn)
    {
        return std::bind(fn,
                         this,
                         std::placeholders::_1,
                         std::placeholders::_2,
                         std::placeholders::_3,
                         std::placeholders::_4);
    }

private:
    void append_trailing_slash(
        const std::smatch &,
        const beast::http::request<beast::http::dynamic_body> &,
        beast::http::response<beast::http::string_body> &);
    void domains(const std::string &, const std::smatch &,
                 const beast::http::request<beast::http::dynamic_body> &,
                 beast::http::response<beast::http::string_body> &);
    void domain(const std::string &, const std::smatch &,
                const beast::http::request<beast::http::dynamic_body> &,
                beast::http::response<beast::http::string_body> &);
    void
    domain_interfaces(const std::string &, const std::smatch &,
                      const beast::http::request<beast::http::dynamic_body> &,
                      beast::http::response<beast::http::string_body> &);

private:
    void virt_connect(virt::connection &, const std::string &,
                      beast::http::response<beast::http::string_body> &);
};

}; // namespace webvirt

#endif /* APP_HPP */
