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
#include "connection.hpp"
#include "../config.hpp"
#include "util.hpp"
#include <chrono>
#include <fmt/format.h>
#include <fstream>
#include <pugixml.hpp>
#include <stdexcept>
#include <thread>
#include <unistd.h>
using namespace webvirt;

virt::connection::connection(const connection &conn)
    : conn_(conn.conn_)
    , errno_(conn.errno_)
{
}

virt::connection::connection(const std::string &uri)
{
    connect(uri);
}

bool virt::connection::operator==(const connection &other) const
{
    return conn_ == other.conn_;
}

bool virt::connection::operator!=(const connection &other) const
{
    return !operator==(other);
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

    auto &lv = libvirt::ref();
    conn_ = lv.virConnectOpen(str.c_str());
    if (!conn_) {
        errno_ = errno;
        std::string message("error: ");
        message.append(this->strerror());
        message.push_back('\n');
        throw std::runtime_error(message);
    }

    return *this;
}

std::vector<virt::domain> virt::connection::domains()
{
    auto &lv = libvirt::ref();
    std::vector<virt::domain> domains_;
    auto domain_ptrs = lv.virConnectListAllDomains(conn_, 0);
    for (auto &domain_ptr : domain_ptrs) {
        domains_.emplace_back(domain_ptr);
    }
    return domains_;
}

std::vector<virt::network> virt::connection::networks()
{
    std::vector<virt::network> networks_;
    auto network_ptrs = libvirt::ref().virConnectListAllNetworks(conn_, 0);
    for (auto &network_ptr : network_ptrs) {
        networks_.emplace_back(network_ptr);
    }
    return networks_;
}

virt::domain virt::connection::domain(const std::string &name)
{
    return virt::domain(get_domain_ptr(name));
}

libvirt::domain_ptr virt::connection::get_domain_ptr(const std::string &name)
{
    auto &lv = libvirt::ref();
    auto domain = lv.virDomainLookupByName(conn_, name.c_str());
    if (!domain) {
        throw std::domain_error("virDomainLookupByName error");
    }
    return domain;
}

libvirt::connect_ptr virt::connection::get_ptr()
{
    return conn_;
}

int virt::connection::error()
{
    return errno_;
}

const char *virt::connection::strerror()
{
    return ::strerror(error());
}
