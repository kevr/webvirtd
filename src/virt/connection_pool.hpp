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
#ifndef VIRT_CONNECTION_POOL_HPP
#define VIRT_CONNECTION_POOL_HPP

#include <virt/connection.hpp>

#include <map>
#include <string>
#include <thread>

namespace webvirt::virt
{

class connection_pool
{
private:
    // username -> connection
    std::mutex connection_mutex_;
    std::map<std::string, connection> connections_;

public:
    connection &get(const std::string &);
};

}; // namespace webvirt::virt

#endif /* VIRT_CONNECTION_POOL_HPP */
