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
#ifndef VIRT_EVENTS_CALLBACKS_LIFECYCLE_HPP
#define VIRT_EVENTS_CALLBACKS_LIFECYCLE_HPP

#include <virt/event_callback.hpp>

namespace webvirt::virt
{

class lifecycle_callback : public event_callback
{
public:
    using event_callback::event_callback;

    typedef void (*function)(webvirt::connect *, webvirt::domain *, int, int,
                             void *);
    lifecycle_callback(function fptr);
    lifecycle_callback(void *);

    virtual event_callback::function function_ptr() const override;

private:
    function real_function_ptr() const;
};

typedef void (*lifecycle_function)(webvirt::connect *, webvirt::domain *, int,
                                   int, void *);

}; // namespace webvirt::virt

#endif /* VIRT_EVENTS_CALLBACKS_LIFECYCLE_HPP */
