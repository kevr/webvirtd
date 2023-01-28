/* Copyright (C) 2023 Kevin Morris <kevr@0cost.org> */
#include "connection.hpp"
#include "util.hpp"
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
        throw std::runtime_error(message);
    } else {
        conn_ = std::shared_ptr<virConnect>(ptr, free_conn());
    }

    return *this;
}

std::vector<std::map<std::string, Json::Value>> virt::connection::domains()
{
    virDomainPtr *domains = nullptr;
    int count;
    count = virConnectListAllDomains(conn_.get(), &domains, 0);
    if (count < 0) {
        throw std::domain_error("virConnectListAllDomains error");
    }

    std::vector<std::map<std::string, Json::Value>> output;
    for (int i = 0; i < count; ++i) {
        std::map<std::string, Json::Value> item;

        const char *name = virDomainGetName(domains[i]);
        item["name"] = name;

        int state = 0, reason = 0;
        virDomainGetState(domains[i], &state, &reason, 0);
        item["state"] = Json::Value(Json::objectValue);
        item["state"]["id"] = state;
        item["state"]["string"] = state_string(state);

        int id = virDomainGetID(domains[i]);
        item["id"] = id;

        output.emplace_back(std::move(item));
    }

    return output;
}

std::map<std::string, Json::Value>
virt::connection::domain(const std::string &name)
{
    std::map<std::string, Json::Value> output;

    virDomainPtr *domains = nullptr;
    int count;
    count = virConnectListAllDomains(conn_.get(), &domains, 0);
    if (count < 0) {
        throw std::domain_error("virConnectListAllDomains error");
    }

    for (int i = 0; i < count; ++i) {
        const char *domain_name = virDomainGetName(domains[i]);
        if (name != domain_name) {
            continue;
        }

        output["name"] = name;

        int state = 0, reason = 0;
        virDomainGetState(domains[i], &state, &reason, 0);
        output["state"] = Json::Value(Json::objectValue);
        output["state"]["id"] = state;
        output["state"]["string"] = state_string(state);

        int id = virDomainGetID(domains[i]);
        output["id"] = id;
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
