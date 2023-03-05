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

using namespace webvirt;
using namespace virt;

lifecycle_callback::lifecycle_callback(function fptr)
    : lifecycle_callback(reinterpret_cast<void *>(fptr))
{
}

lifecycle_callback::lifecycle_callback(void *fptr)
    : event_callback(fptr)
{
}

event_callback::function lifecycle_callback::function_ptr() const
{
    return add_event_callback(VIR_DOMAIN_EVENT_ID_LIFECYCLE,
                              VIR_DOMAIN_EVENT_CALLBACK(real_function_ptr()));
}

lifecycle_callback::function lifecycle_callback::real_function_ptr() const
{
    return reinterpret_cast<function>(fptr_);
}
