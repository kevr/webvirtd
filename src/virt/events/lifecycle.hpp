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
#ifndef VIRT_EVENTS_LIFECYCLE_HPP
#define VIRT_EVENTS_LIFECYCLE_HPP

#include <http/handlers.hpp>
#include <virt/event.hpp>
#include <virt/events/callbacks/lifecycle.hpp>
#include <ws/pool.hpp>

namespace webvirt::virt
{

class lifecycle_event : public event
{
    using handler =
        http::handler<virt::connection &, virt::domain &, int, int>;
    handler on_event_;

public:
    using event::event;
    lifecycle_event(virt::connection &, const lifecycle_callback &,
                    handler::type on_event);

    static constexpr int id()
    {
        return VIR_DOMAIN_EVENT_ID_LIFECYCLE;
    }

    void register_event(int event_id, const virt::event_callback &);

    handler_setter(on_event, on_event_);

    friend void on_event_handler(webvirt::connect *, webvirt::domain *, int,
                                 int, void *);
    static void on_event_handler(webvirt::connect *, webvirt::domain *, int,
                                 int, void *);
};

}; // namespace webvirt::virt

#endif /* VIRT_EVENTS_LIFECYCLE_HPP */
