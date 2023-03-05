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
#ifndef VIRT_EVENT_CALLBACK_HPP
#define VIRT_EVENT_CALLBACK_HPP

#include <libvirt.hpp>

namespace webvirt::virt
{

class event_callback
{
protected:
    void *fptr_ { nullptr };

protected:
    event_callback(void *fptr);

public:
    virtual ~event_callback() = default;

    typedef void (*function)(webvirt::connect *, webvirt::domain *, void *);
    virtual function function_ptr() const = 0;
};

typedef void (*event_function)(webvirt::connect *, webvirt::domain *, void *);
event_function add_event_callback(int, event_function);
event_function get_event_callback(int);

}; // namespace webvirt::virt

#endif /* VIRT_EVENT_CALLBACK_HPP */
