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

#include <libvirt/libvirt.h>
#include <memory>
#include <string>
#include <vector>

namespace webvirt
{

class libvirt
{
public:
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

private:
    static libvirt instance_;
    static libvirt *ptr_;

public:
    static libvirt &ref();
    static libvirt &change(libvirt &);
    static libvirt &reset();

private:
    struct free_connect_ptr {
        void operator()(virConnectPtr);
    };

    struct free_domain_ptr {
        void operator()(virDomainPtr);
    };

public:
    using connect_ptr = std::shared_ptr<connect>;
    using domain_ptr = std::shared_ptr<domain>;
    using block_info_ptr = std::shared_ptr<block_info>;

public:
    virtual ~libvirt() = default;
    virtual connect_ptr virConnectOpen(const char *);
    virtual domain_ptr virDomainLookupByName(connect_ptr, const char *);
    virtual std::string virDomainGetXMLDesc(domain_ptr, int);
    virtual block_info_ptr virDomainGetBlockInfo(domain_ptr, const char *,
                                                 int);
    virtual int virDomainGetAutostart(domain_ptr, int *);
    virtual int virDomainSetAutostart(domain_ptr, int);
    virtual int virDomainCreate(domain_ptr);
    virtual int virDomainShutdown(domain_ptr);
    virtual int virDomainGetState(domain_ptr, int *, int *, int);
    virtual int virDomainGetID(domain_ptr);
    virtual const char *virDomainGetName(domain_ptr);
    virtual std::vector<domain_ptr> virConnectListAllDomains(connect_ptr, int);
};

}; // namespace webvirt

#endif /* LIBVIRT_HPP */
