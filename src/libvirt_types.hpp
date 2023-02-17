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
#ifndef LIBVIRT_TYPES_HPP
#define LIBVIRT_TYPES_HPP

#include <memory>

#ifndef TEST_BUILD
#include <libvirt/libvirt.h>
#include <libvirt/virterror.h>
#endif

namespace webvirt
{

#ifdef TEST_BUILD
struct connect {
};
struct domain {
};
struct network {
};
struct block_info {
    unsigned long capacity;
    unsigned long allocation;
    unsigned long physical;
};

struct error__ {
    char *message;
};
using error_ = error__ *;

#else
using connect = virConnect;
using domain = virDomain;
using network = virNetwork;
using block_info = virDomainBlockInfo;
using error_ = virErrorPtr;
#endif

using connect_ptr = std::shared_ptr<connect>;
using domain_ptr = std::shared_ptr<domain>;
using network_ptr = std::shared_ptr<network>;
using block_info_ptr = std::shared_ptr<block_info>;

using error_function = void (*)(void *, error_);

}; // namespace webvirt

#endif /* LIBVIRT_TYPES_HPP */
