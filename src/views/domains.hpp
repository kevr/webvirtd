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
#ifndef VIEWS_DOMAINS_HPP
#define VIEWS_DOMAINS_HPP

#include <http/namespaces.hpp>
#include <virt/connection.hpp>
#include <virt/domain.hpp>

#include <regex>

namespace webvirt::views
{

class domains
{
public:
    void index(virt::connection &, const std::smatch &, const http::request &,
               http::response &);
    void show(virt::connection &, virt::domain, const std::smatch &,
              const http::request &, http::response &);
    void autostart(virt::connection &, virt::domain, const std::smatch &,
                   const http::request &, http::response &);
    void metadata(virt::connection &, virt::domain, const std::smatch &,
                  const http::request &, http::response &);
    void bootmenu(virt::connection &, virt::domain, const std::smatch &,
                  const http::request &, http::response &);
    void start(virt::connection &, virt::domain, const std::smatch &,
               const http::request &, http::response &);
    void shutdown(virt::connection &, virt::domain, const std::smatch &,
                  const http::request &, http::response &);
};

}; // namespace webvirt::views

#endif /* VIEWS_DOMAINS_HPP */
