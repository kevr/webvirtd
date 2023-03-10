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
#ifndef LIBVIRT_HPP
#define LIBVIRT_HPP

#include <libvirt_types.hpp>
#include <singleton.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

#ifdef TEST_BUILD
#include <stubs/libvirt.hpp>
#endif

namespace webvirt
{

/** A libvirt library call wrapper */
class libvirt : public singleton<libvirt>
{
public:
    /** connect_ptr destruction functor */
    struct free_connect_ptr {
        void operator()(connect *);
    };

    /** domain_ptr destruction functor */
    struct free_domain_ptr {
        void operator()(domain *);
    };

    /** network_ptr destruction functor */
    struct free_network_ptr {
        void operator()(network *);
    };

public:
    /** Default virtual destructor */
    virtual ~libvirt() = default;

    // virConnect
    virtual connect_ptr virConnectOpen(const char *);
    virtual int virConnectRegisterCloseCallback(connect_ptr,
                                                void (*)(connect *, int,
                                                         void *),
                                                void *, void (*)(void *));
    virtual std::string virConnectGetCapabilities(connect_ptr);
    virtual std::string virConnectGetHostname(connect_ptr);
    virtual int virConnectGetLibVersion(connect_ptr, unsigned long *);
    virtual int virConnectGetMaxVcpus(connect_ptr, const char *);
    virtual std::string virConnectGetSysinfo(connect_ptr, unsigned int);
    virtual const char *virConnectGetType(connect_ptr);
    virtual std::string virConnectGetURI(connect_ptr);
    virtual int virConnectGetVersion(connect_ptr, unsigned long *);
    virtual int virConnectIsEncrypted(connect_ptr);
    virtual int virConnectIsSecure(connect_ptr);
    virtual std::vector<domain_ptr> virConnectListAllDomains(connect_ptr, int);
    virtual std::vector<network_ptr> virConnectListAllNetworks(connect_ptr,
                                                               int);

    // virDomain
    virtual domain_ptr virDomainLookupByName(connect_ptr, const char *);
    virtual int virDomainCreate(domain_ptr);
    virtual int virDomainRef(webvirt::domain *);
    virtual int virConnectDomainEventRegisterAny(
        webvirt::connect *, webvirt::domain *, int,
        void (*)(webvirt::connect *, webvirt::domain *, void *), void *,
        void (*)(void *));
    virtual int virConnectDomainEventDeregisterAny(connect_ptr, int);
    virtual int virDomainGetState(domain_ptr, int *, int *, int);
    virtual int virDomainGetID(domain_ptr);
    virtual const char *virDomainGetName(domain_ptr);
    virtual int virDomainGetAutostart(domain_ptr, int *);
    virtual int virDomainSetAutostart(domain_ptr, int);
    virtual std::string virDomainGetMetadata(domain_ptr, int, const char *,
                                             unsigned int);
    virtual int virDomainSetMetadata(domain_ptr, int, const char *,
                                     const char *, const char *, unsigned int);
    virtual std::string virDomainGetXMLDesc(domain_ptr, int);
    virtual domain_ptr virDomainDefineXML(connect_ptr, const char *);
    virtual block_info_ptr virDomainGetBlockInfo(domain_ptr, const char *,
                                                 int);
    virtual int virDomainShutdown(domain_ptr);

    // virNetwork
    virtual std::string virNetworkGetXMLDesc(network_ptr, unsigned int);

    // virEvent
    virtual int virEventRegisterDefaultImpl();
    virtual int virEventAddTimeout(int, void (*)(int, void *), void *,
                                   void (*)(void *));
    virtual int virEventRunDefaultImpl();

    // virterror
    virtual void virConnSetErrorFunc(connect_ptr, void *,
                                     webvirt::error_function);
};

}; // namespace webvirt

#endif /* LIBVIRT_HPP */
