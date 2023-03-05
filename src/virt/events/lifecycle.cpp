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
#include <virt/events/callbacks/lifecycle.hpp>
#include <virt/events/lifecycle.hpp>

using namespace webvirt;
using namespace virt;

lifecycle_event::lifecycle_event(virt::connection &conn,
                                 const lifecycle_callback &cb,
                                 handler::type on_event)
    : event(conn)
{
    this->on_event(on_event);
    register_event(VIR_DOMAIN_EVENT_ID_LIFECYCLE, cb);
}

void lifecycle_event::register_event(int event_id,
                                     const virt::event_callback &cb)
{
    callback_id_ = libvirt::ref().virConnectDomainEventRegisterAny(
        conn_.get_ptr().get(),
        nullptr,
        event_id,
        cb.function_ptr(),
        reinterpret_cast<void *>(this),
        nullptr);

    if (callback_id_ == -1) {
        throw std::runtime_error("Event registration failed");
    }
}

void lifecycle_event::on_event_handler(webvirt::connect *,
                                       webvirt::domain *dptr, int type,
                                       int detail, void *opaque)
{
    auto ev = reinterpret_cast<lifecycle_event *>(opaque);

    // Construct a new virt::domain based on `dptr`
    libvirt::ref().virDomainRef(dptr);
    webvirt::domain_ptr dptr_(dptr, libvirt::free_domain_ptr());
    virt::domain domain(std::move(dptr_));

    ev->on_event_(ev->conn_, domain, type, detail);
}
