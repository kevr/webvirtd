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
#include <libvirt.hpp>
#include <virt/event.hpp>

#include <map>

using namespace webvirt;
using namespace virt;

/* A no-op callback used for default event timeouts */
void webvirt::virt::event_timeout(int, void *)
{
}

/* A no-op callback used for default event timeouts **/
void webvirt::virt::event_timeout_data_free(void *)
{
}

int event::register_impl()
{
    auto &lv = libvirt::ref();
    if (lv.virEventRegisterDefaultImpl() == -1)
        return -1;

    // Add a timeout, by default, of 20 milliseconds for each
    // event loop iteration.
    return lv.virEventAddTimeout(
        20, &event_timeout, nullptr, &event_timeout_data_free);
}

int event::run_one()
{
    return libvirt::ref().virEventRunDefaultImpl();
}

event::event(virt::connection &conn)
    : conn_(conn)
{
}

event::~event()
{
    libvirt::ref().virConnectDomainEventDeregisterAny(conn_.get_ptr(),
                                                      callback_id_);
}
