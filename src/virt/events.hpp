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
#ifndef VIRT_EVENTS_HPP
#define VIRT_EVENTS_HPP

#include <http/handlers.hpp>
#include <virt/event.hpp>

#include <map>
#include <memory>
#include <thread>

namespace webvirt::virt
{

/** A collection of webvirt::virt::event objects */
class events
{
    std::mutex events_mutex_;
    std::map<int, event_ptr> events_;

public:
    /** Emplace a new std::shared<event> with constructor arguments
     *
     * @param key Container key to emplace
     * @param args Variadic arguments forwarded to std::make_shared
     **/
    template <typename... Args>
    void emplace(int key, Args &&...args)
    {
        events_.emplace(key,
                        std::make_shared<event>(std::forward<Args>(args)...));
    }

    /** Append an event_ptr to internal event container
     *
     * @param key Container key to set
     * @param ev Valid std::shared_ptr<event>
     **/
    void set(int, event_ptr &&);

    /** Return virt::event mapped to event_id
     *
     * @param event_id libvirt event id
     * @returns Reference to virt::event mapped to event_id
     **/
    virt::event &get(int);

    /** Remove an event_ptr from the internal event container
     *
     * @param key Container key to remove
     **/
    void remove(int);

    /** Get the size of internal events mapping
     *
     * @returns Size of internal events mapping
     **/
    std::size_t size();
};

}; // namespace webvirt::virt

#endif /* VIRT_EVENTS_HPP */
