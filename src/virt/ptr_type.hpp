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
#ifndef VIRT_PTR_TYPE_HPP
#define VIRT_PTR_TYPE_HPP

#include <utility>

namespace webvirt::virt
{

template <typename ptr_t>
class ptr_type
{
protected:
    ptr_t ptr_;

public:
    ptr_type() = default;

    ptr_type(ptr_t ptr)
        : ptr_(std::move(ptr))
    {
    }

    ptr_type(const ptr_type &o)
        : ptr_(o.ptr_)
    {
    }

    ptr_type(ptr_type &&o)
        : ptr_(std::move(o.ptr_))
    {
    }

    ptr_type &operator=(ptr_type o)
    {
        ptr_ = std::move(o.ptr_);
        return *this;
    }

    operator bool() const
    {
        return ptr_ != nullptr;
    }
};

}; // namespace webvirt::virt

#endif /* VIRT_PTR_TYPE_HPP */
