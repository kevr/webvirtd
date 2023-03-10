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

#include <libvirt.hpp>

#include <gmock/gmock.h>

namespace webvirt::mocks
{

class libvirt_mock : public webvirt::libvirt
{
public:
    ~libvirt_mock() = default;

    MOCK_METHOD(connect_ptr, virConnectOpen, (const char *));
    MOCK_METHOD(int, virConnectRegisterCloseCallback,
                (connect_ptr, void (*)(webvirt::connect *, int, void *),
                 void *, void (*)(void *)));

    MOCK_METHOD(std::string, virConnectGetCapabilities, (connect_ptr));
    MOCK_METHOD(std::string, virConnectGetHostname, (connect_ptr));
    MOCK_METHOD(int, virConnectGetLibVersion, (connect_ptr, unsigned long *));
    MOCK_METHOD(int, virConnectGetMaxVcpus, (connect_ptr, const char *));
    MOCK_METHOD(std::string, virConnectGetSysinfo,
                (connect_ptr, unsigned int));
    MOCK_METHOD(const char *, virConnectGetType, (connect_ptr));

    MOCK_METHOD(std::string, virConnectGetURI, (connect_ptr));
    MOCK_METHOD(int, virConnectGetVersion, (connect_ptr, unsigned long *));
    MOCK_METHOD(int, virConnectIsEncrypted, (connect_ptr));
    MOCK_METHOD(int, virConnectIsSecure, (connect_ptr));

    MOCK_METHOD(std::vector<domain_ptr>, virConnectListAllDomains,
                (connect_ptr, int));
    MOCK_METHOD(std::vector<network_ptr>, virConnectListAllNetworks,
                (connect_ptr, int));
    MOCK_METHOD(domain_ptr, virDomainLookupByName,
                (connect_ptr, const char *));
    MOCK_METHOD(int, virDomainCreate, (domain_ptr));
    MOCK_METHOD(int, virConnectDomainEventRegisterAny,
                (webvirt::connect *, webvirt::domain *, int,
                 void (*)(webvirt::connect *, webvirt::domain *, void *),
                 void *, void (*)(void *)));
    MOCK_METHOD(block_info_ptr, virDomainGetBlockInfo,
                (domain_ptr, const char *, int));
    MOCK_METHOD(int, virDomainGetAutostart, (domain_ptr, int *));
    MOCK_METHOD(int, virDomainSetAutostart, (domain_ptr, int));
    MOCK_METHOD(std::string, virDomainGetMetadata,
                (domain_ptr, int, const char *, unsigned int));
    MOCK_METHOD(int, virDomainSetMetadata,
                (domain_ptr, int, const char *, const char *, const char *,
                 unsigned int));
    MOCK_METHOD(int, virDomainShutdown, (domain_ptr));
    MOCK_METHOD(std::string, virDomainGetXMLDesc, (domain_ptr, int));
    MOCK_METHOD(int, virDomainGetState, (domain_ptr, int *, int *, int));
    MOCK_METHOD(int, virDomainGetID, (domain_ptr));
    MOCK_METHOD(const char *, virDomainGetName, (domain_ptr));
    MOCK_METHOD(domain_ptr, virDomainDefineXML, (connect_ptr, const char *));

    MOCK_METHOD(std::string, virNetworkGetXMLDesc,
                (network_ptr, unsigned int));

    MOCK_METHOD(int, virEventRegisterDefaultImpl, ());
    MOCK_METHOD(int, virEventRunDefaultImpl, ());
};

// using libvirt = testing::NiceMock<libvirt_mock>;
using libvirt = libvirt_mock;

}; // namespace webvirt::mocks

#endif /* MOCKS_LIBVIRT_HPP */
