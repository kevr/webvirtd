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
#ifndef MOCKS_LIBVIRT_HPP
#define MOCKS_LIBVIRT_HPP

#include "../libvirt.hpp"
#include <gmock/gmock.h>

namespace webvirt::mocks
{

class libvirt : public webvirt::libvirt
{
public:
    ~libvirt() = default;

    MOCK_METHOD(connect_ptr, virConnectOpen, (const char *));
    MOCK_METHOD(std::vector<domain_ptr>, virConnectListAllDomains,
                (connect_ptr, int));
    MOCK_METHOD(domain_ptr, virDomainLookupByName,
                (connect_ptr, const char *));
    MOCK_METHOD(std::shared_ptr<char>, virDomainGetXMLDesc, (domain_ptr, int));
    MOCK_METHOD(int, virDomainGetState, (domain_ptr, int *, int *, int));
    MOCK_METHOD(int, virDomainGetID, (domain_ptr));
    MOCK_METHOD(const char *, virDomainGetName, (domain_ptr));
};

}; // namespace webvirt::mocks

#endif /* MOCKS_LIBVIRT_HPP */
