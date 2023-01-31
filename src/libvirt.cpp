/* SPDX-License-Identifier: Apache 2.0 */
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

void libvirt::free_connect_ptr::operator()(virConnectPtr ptr)
{
    if (ptr) {
        virConnectClose(ptr);
    }
}

void libvirt::free_domain_ptr::operator()(virDomainPtr ptr)
{
    if (ptr) {
        virDomainFree(ptr);
    }
}

libvirt::connect_ptr libvirt::virConnectOpen(const char *uri)
{
    return connect_ptr(::virConnectOpen(uri), free_connect_ptr());
}

libvirt::domain_ptr libvirt::virDomainLookupByName(connect_ptr conn,
                                                   const char *name)
{
    return domain_ptr(::virDomainLookupByName(conn.get(), name),
                      free_domain_ptr());
}

std::shared_ptr<char> libvirt::virDomainGetXMLDesc(domain_ptr domain,
                                                   int flags)
{
    return std::shared_ptr<char>(::virDomainGetXMLDesc(domain.get(), flags));
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

std::vector<libvirt::domain_ptr>
libvirt::virConnectListAllDomains(connect_ptr conn, int flags)
{
    std::vector<libvirt::domain_ptr> output;

    virDomainPtr *domains = nullptr;
    int count = ::virConnectListAllDomains(conn.get(), &domains, flags);
    for (int i = 0; i < count; ++i) {
        output.emplace_back(domains[i], free_domain_ptr());
    }

    return output;
}
