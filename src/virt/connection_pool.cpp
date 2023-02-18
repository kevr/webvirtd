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
#include <util/bench.hpp>
#include <util/logging.hpp>
#include <virt/connection_pool.hpp>
#include <virt/util.hpp>

using namespace webvirt::virt;

connection &connection_pool::get(const std::string &user)
{
    bench<double> bench_;

    auto iter = connections_.find(user);
    if (iter == connections_.end()) {
        connections_[user] = virt::connection();
        connections_[user].connect(virt::uri(user));
        auto ms = bench_.end() * 1000;
        logger::debug(fmt::format("Connected to libvirt in {}ms", int(ms)));
        return connections_.at(user);
    }

    if (!iter->second) {
        // If connection is stale, try reconnecting.
        iter->second = virt::connection();
        iter->second.connect(virt::uri(user));
        auto ms = bench_.end() * 1000;
        logger::debug(fmt::format("Reconnected to libvirt in {}ms", int(ms)));
    }

    return iter->second;
}
