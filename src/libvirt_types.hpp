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

#ifndef TEST_BUILD
#include <libvirt/libvirt.h>
#endif

#include <memory>

namespace webvirt
{
#ifdef TEST_BUILD
struct connect {
};
struct domain {
};
struct block_info {
    unsigned long capacity;
    unsigned long allocation;
    unsigned long physical;
};
#else
using connect = virConnect;
using domain = virDomain;
using block_info = virDomainBlockInfo;
#endif
}; // namespace webvirt

#endif /* LIBVIRT_TYPES_HPP */
