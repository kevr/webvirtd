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
#include <util/json.hpp>
#include <ws/pool.hpp>

using namespace webvirt::websocket;

pool &pool::add(const std::string &user, connection_ptr conn)
{
    std::lock_guard<std::mutex> guard(mutex_);
    map_[user].emplace_back(std::move(conn));
    return *this;
}

pool &pool::remove(const std::string &user, connection_ptr conn)
{
    std::lock_guard<std::mutex> guard(mutex_);
    auto &v = map_[user];
    v.erase(std::find_if(v.begin(), v.end(), [conn](auto &c) {
        return c == conn;
    }));
    return *this;
}

std::list<connection_ptr> &pool::operator[](const std::string &key)
{
    return map_[key];
}

void pool::broadcast(const std::string &user, Json::Value data)
{
    std::lock_guard<std::mutex> guard(mutex_);
    for (auto &ws : map_[user]) {
        ws->write(json::stringify(data));
    }
}
