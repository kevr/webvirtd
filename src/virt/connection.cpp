/* Copyright (C) 2023 Kevin Morris <kevr@0cost.org> */
#include "connection.hpp"
#include <iostream>
#include <stdexcept>
using namespace webvirt;

void virt::connection::free_conn::operator()(virConnectPtr ptr)
{
    virConnectClose(ptr);
}

virt::connection::connection(const connection &conn)
    : conn_(conn.conn_)
    , errno_(conn.errno_)
{
}

virt::connection::connection(const std::string &uri)
{
    connect(uri);
}

virt::connection &virt::connection::operator=(const virt::connection &conn)
{
    conn_ = conn.conn_;
    errno_ = conn.errno_;
    return *this;
}

virt::connection &virt::connection::connect(const std::string &str)
{
    if (conn_) {
        throw std::overflow_error("cannot connect more than once");
    }

    auto *ptr = virConnectOpen(str.c_str());
    if (!ptr) {
        errno_ = errno;
        std::string message("error: ");
        message.append(this->strerror());
        message.push_back('\n');
        std::cerr << message;
    } else {
        conn_ = std::shared_ptr<virConnect>(ptr, free_conn());
    }

    return *this;
}

std::vector<std::string> virt::connection::domains()
{
    virDomainPtr *domains = nullptr;
    int count;
    count = virConnectListAllDomains(conn_.get(), &domains, 0);
    if (count < 0) {
        throw std::domain_error("virConnectListAllDomains error");
    }

    std::vector<std::string> output;
    for (int i = 0; i < count; ++i) {
        const char *name = virDomainGetName(domains[i]);
        output.emplace_back(name);
    }

    return output;
}

int virt::connection::error()
{
    return errno_;
}

const char *virt::connection::strerror()
{
    return ::strerror(error());
}
