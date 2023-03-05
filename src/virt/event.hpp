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
#ifndef VIRT_EVENT_HPP
#define VIRT_EVENT_HPP

#include <http/handlers.hpp>
#include <util/signal.hpp>
#include <virt/connection.hpp>
#include <virt/domain.hpp>
#include <virt/event_callback.hpp>

#include <memory>

namespace webvirt::virt
{

/** Libvirt event timeout handler */
void event_timeout(int, void *);

/** Libvirt event timeout opaque data destructor */
void event_timeout_data_free(void *);

/** Libvirt event object
 *
 * Libvirt events should be initialized using event::register_impl(),
 * and the event loop can be processed one iteration at a time using
 * event::run_one.
 **/
class event
{
protected:
    virt::connection &conn_;
    int callback_id_ { -1 };

public:
    /** Register libvirt default event implementation
     *
     * @returns 0 on success, -1 on error
     * **/
    static int register_impl();

    /** Run one iteration of the event processing loop
     *
     * @returns 0 on success, -1 on error
     **/
    static int run_one();

public:
    /** Construct an event handler
     *
     * This constructor binds the passed libvirt connection and domain
     * to internal references, which are then accessed by internal
     * libvirt event handler.
     *
     * Therefore, this event **must** not outlive the connection nor domain
     * object which it depends on.
     *
     * @param conn libvirt connection
     * @param event_id virDomainEventID
     **/
    event(virt::connection &);

    /** Destruct this event */
    virtual ~event();
};

using event_ptr = std::shared_ptr<event>;

}; // namespace webvirt::virt

#endif /* VIRT_EVENT_HPP */
