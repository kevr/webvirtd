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
#ifndef WS_POOL_HPP
#define WS_POOL_HPP

#include <ws/connection.hpp>

#include <list>
#include <map>
#include <mutex>
#include <string>

namespace webvirt::websocket
{

class pool
{
    std::mutex mutex_;
    std::map<std::string, std::list<connection_ptr>> map_;

public:
    pool &add(const std::string &, connection_ptr);
    pool &remove(const std::string &, connection_ptr);
};

using pool_ptr = std::shared_ptr<pool>;

}; // namespace webvirt::websocket

#endif /* WS_POOL_HPP */
