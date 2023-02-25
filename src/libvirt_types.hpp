/*
 * When TEST_BUILD is defined, stubbed libvirt structures are defined in this
 * file. This allows us to bypass some whacky libvirt definitions when testing
 * with mocks.
 *
 * - connect: virConnect
 * - domain: virDomain
 * - block_info: virDomainBlockInfo
 * - network: virNetwork
 * - error_: virErrorPtr
 * - connect_ptr: std::shared_ptr<connect>
 * - domain_ptr: std::shared_ptr<domain>
 * - block_info_ptr: std::shared_ptr<block_info>
 * - network_ptr: std::shared_ptr<network>
 *
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

/** virConnect stub */
struct connect {
};

/** virDomain stub */
struct domain {
};

/** virDomainBlockInfo stub */
struct block_info {
    unsigned long capacity;
    unsigned long allocation;
    unsigned long physical;
};

/** virNetwork stub */
struct network {
};

/** virError stub */
struct error__ {
    char *message;
};

/** virErrorPtr stub */
using error_ = error__ *;

#else /* TEST_BUILD not defined */

using connect = virConnect;
using domain = virDomain;
using block_info = virDomainBlockInfo;
using network = virNetwork;
using error_ = virErrorPtr;

#endif

using connect_ptr = std::shared_ptr<connect>;
using domain_ptr = std::shared_ptr<domain>;
using block_info_ptr = std::shared_ptr<block_info>;
using network_ptr = std::shared_ptr<network>;

using error_function = void (*)(void *, error_);

}; // namespace webvirt

#endif /* LIBVIRT_TYPES_HPP */
