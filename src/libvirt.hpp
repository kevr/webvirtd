/* SPDX-License-Identifier: Apache 2.0 */
#ifndef LIBVIRT_HPP
#define LIBVIRT_HPP

#include <libvirt/libvirt.h>
#include <memory>
#include <vector>

namespace webvirt
{

class libvirt
{
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
    using connect_ptr = std::shared_ptr<virConnect>;
    using domain_ptr = std::shared_ptr<virDomain>;

public:
    virtual ~libvirt() = default;
    virtual connect_ptr virConnectOpen(const char *);
    virtual domain_ptr virDomainLookupByName(connect_ptr, const char *);
    virtual std::shared_ptr<char> virDomainGetXMLDesc(domain_ptr, int);
    virtual int virDomainGetState(domain_ptr, int *, int *, int);
    virtual int virDomainGetID(domain_ptr);
    virtual const char *virDomainGetName(domain_ptr);
    virtual std::vector<domain_ptr> virConnectListAllDomains(connect_ptr, int);
};

}; // namespace webvirt

#endif /* LIBVIRT_HPP */
