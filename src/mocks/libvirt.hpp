/* Copyright (C) 2023 Kevin Morris <kevr@0cost.org> */
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
    MOCK_METHOD(domain_ptr, virDomainLookupByName,
                (connect_ptr, const char *));
    MOCK_METHOD(std::shared_ptr<char>, virDomainGetXMLDesc, (domain_ptr, int));
    MOCK_METHOD(int, virDomainGetState, (domain_ptr, int *, int *, int));
    MOCK_METHOD(int, virDomainGetID, (domain_ptr));
    MOCK_METHOD(const char *, virDomainGetName, (domain_ptr));
    MOCK_METHOD(std::vector<domain_ptr>, virConnectListAllDomains,
                (connect_ptr, int));
};

}; // namespace webvirt::mocks

#endif /* MOCKS_LIBVIRT_HPP */
