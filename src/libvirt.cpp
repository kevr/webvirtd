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
#include "libvirt.hpp"
using namespace webvirt;

libvirt libvirt::instance_;
libvirt *libvirt::ptr_ = &instance_;

libvirt &libvirt::ref()
{
    return *ptr_;
}

libvirt &libvirt::change(libvirt &other)
{
    ptr_ = &other;
    return ref();
}

libvirt &libvirt::reset()
{
    ptr_ = &instance_;
    return ref();
}

void libvirt::free_connect_ptr::operator()(connect *ptr)
{
    if (ptr) {
        ::virConnectClose(ptr);
    }
}

void libvirt::free_domain_ptr::operator()(domain *ptr)
{
    if (ptr) {
        ::virDomainFree(ptr);
    }
}

/* virConnect definitions */
libvirt::connect_ptr libvirt::virConnectOpen(const char *uri)
{
    return connect_ptr(::virConnectOpen(uri), free_connect_ptr());
}

std::vector<libvirt::domain_ptr>
libvirt::virConnectListAllDomains(connect_ptr conn, int flags)
{
    std::vector<libvirt::domain_ptr> output;

    domain **domains = nullptr;
    int count = ::virConnectListAllDomains(conn.get(), &domains, flags);
    for (int i = 0; i < count; ++i) {
        output.emplace_back(domains[i], free_domain_ptr());
    }

    return output;
}

/* virDomain definitions */
libvirt::domain_ptr libvirt::virDomainLookupByName(connect_ptr conn,
                                                   const char *name)
{
    return domain_ptr(::virDomainLookupByName(conn.get(), name),
                      free_domain_ptr());
}

int libvirt::virDomainCreate(domain_ptr domain)
{
    return ::virDomainCreate(domain.get());
}

int libvirt::virDomainGetState(domain_ptr domain, int *state, int *reason,
                               int flags)
{
    return ::virDomainGetState(domain.get(), state, reason, flags);
}

int libvirt::virDomainGetID(domain_ptr domain)
{
    return ::virDomainGetID(domain.get());
}

const char *libvirt::virDomainGetName(domain_ptr domain)
{
    return ::virDomainGetName(domain.get());
}

int libvirt::virDomainGetAutostart(domain_ptr domain, int *autostart)
{
    return ::virDomainGetAutostart(domain.get(), autostart);
}

int libvirt::virDomainSetAutostart(domain_ptr domain, int autostart)
{
    return ::virDomainSetAutostart(domain.get(), autostart);
}

std::string libvirt::virDomainGetMetadata(domain_ptr domain, int type,
                                          const char *uri, unsigned int flags)
{
    std::string output;
    char *value = ::virDomainGetMetadata(domain.get(), type, uri, flags);
    if (value) {
        output = std::string(value);
    }
    free(value);
    return output;
}

int libvirt::virDomainSetMetadata(domain_ptr domain, int type,
                                  const char *metadata, const char *key,
                                  const char *uri, unsigned int flags)
{
    return ::virDomainSetMetadata(
        domain.get(), type, metadata, key, uri, flags);
}

std::string libvirt::virDomainGetXMLDesc(domain_ptr domain, int flags)
{
    auto *buffer = ::virDomainGetXMLDesc(domain.get(), flags);
    std::string s(buffer);
    free(buffer);
    return s;
}

libvirt::domain_ptr libvirt::virDomainDefineXML(connect_ptr conn,
                                                const char *xml)
{
    auto ptr = ::virDomainDefineXML(conn.get(), xml);
    return std::shared_ptr<domain>(ptr, free_domain_ptr());
}

libvirt::block_info_ptr
libvirt::virDomainGetBlockInfo(domain_ptr domain, const char *name, int flags)
{
    auto block_info_ptr = std::make_shared<block_info>();
    ::virDomainGetBlockInfo(domain.get(), name, block_info_ptr.get(), flags);
    return block_info_ptr;
}

int libvirt::virDomainShutdown(domain_ptr domain)
{
    return ::virDomainShutdown(domain.get());
}
