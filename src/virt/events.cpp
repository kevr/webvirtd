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
#include <virt/events.hpp>

using namespace webvirt;
using namespace virt;

void events::set(int key, event_ptr &&ev)
{
    std::lock_guard<std::mutex> guard(events_mutex_);
    events_.emplace(key, std::move(ev));
}

virt::event &events::get(int event_id)
{
    std::lock_guard<std::mutex> guard(events_mutex_);
    return *events_.at(event_id);
}

void events::remove(int key)
{
    std::lock_guard<std::mutex> guard(events_mutex_);
    events_.erase(events_.find(key));
}

std::size_t events::size()
{
    std::lock_guard<std::mutex> guard(events_mutex_);
    return events_.size();
}
