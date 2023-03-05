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
#include <virt/event_callback.hpp>

#include <map>

using namespace webvirt;
using namespace virt;

event_callback::event_callback(void *fptr)
    : fptr_(fptr)
{
}

std::map<int, void (*)(webvirt::connect *, webvirt::domain *, void *)>
    event_callbacks;

event_function virt::add_event_callback(int type, event_function fn)
{
    event_callbacks[type] = fn;
    return fn;
}

event_function virt::get_event_callback(int type)
{
    return event_callbacks.at(type);
}
