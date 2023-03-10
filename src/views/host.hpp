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
#ifndef VIEWS_HOST_HPP
#define VIEWS_HOST_HPP

#include <http/connection.hpp>
#include <http/types.hpp>
#include <virt/connection.hpp>

#include <regex>

namespace webvirt::views
{

/** HTTP views related to a libvirt host */
class host
{
public:
    /** Show libvirt host information
     *
     * @param conn libvirt connection
     * @param http_conn HTTP connection
     * @param location Request URI regex match
     * @param request http::request
     * @param response http::response
     **/
    void show(virt::connection &, http::connection_ptr, const std::smatch &,
              const http::request &, http::response &);

    /** List libvirt host networks
     *
     * @param conn libvirt connection
     * @param http_conn HTTP connection
     * @param location Request URI regex match
     * @param request http::request
     * @param response http::response
     **/
    void networks(virt::connection &, http::connection_ptr,
                  const std::smatch &, const http::request &,
                  http::response &);
};

}; // namespace webvirt::views

#endif /* VIEWS_HOST_HPP */
