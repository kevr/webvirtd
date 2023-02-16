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
#ifndef HTTP_ROUTER_HPP
#define HTTP_ROUTER_HPP

#include "../logging.hpp"
#include "../virt/connection.hpp"
#include "namespaces.hpp"
#include <boost/beast.hpp>
#include <map>
#include <regex>
#include <string>
#include <vector>

namespace webvirt::http
{

class router
{
private:
    std::map<std::string, route_function> routes_;
    std::map<std::string, std::regex> regex_;

public:
    void run(const http::request &, http::response &);
    void route(const std::string &, route_function);
};

}; // namespace webvirt::http

#endif /* HTTP_ROUTER_HPP */
