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
#include <util/config.hpp>
#include <util/retry.hpp>
#include <virt/connection.hpp>
#include <virt/util.hpp>

#include <list>

using namespace webvirt;

#ifdef TEST_BUILD
bool &virt::connection::closed()
{
    return closed_;
}
#else
bool virt::connection::closed()
{
    return closed_;
}
#endif

virt::connection::connection(const connection &conn)
    : conn_(conn.conn_)
    , errno_(conn.errno_)
    , closed_(conn.closed_)
    , user_(conn.user_)
{
}

virt::connection::connection(const std::string &user)
{
    connect(user);
}

virt::connection::operator bool()
{
    return !closed_;
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
    closed_ = conn.closed_;
    user_ = conn.user_;
    return *this;
}

const std::string &virt::connection::user() const
{
    return user_;
}

virt::connection &virt::connection::connect(const std::string &user)
{
    logger::debug("Connecting to libvirt");

    if (conn_) {
        throw std::overflow_error("cannot connect more than once");
    }

    auto &lv = libvirt::ref();
    const std::string uri(virt::uri(user));
    conn_ = lv.virConnectOpen(uri.c_str());
    if (!conn_) {
        errno_ = errno;
        std::string message("error: ");
        message.append(this->strerror());
        message.push_back('\n');
        throw std::runtime_error(message);
    }
    user_ = user;
    closed_ = false;

    auto close_func = [](webvirt::connect *, int, void *data) {
        bool *closed = reinterpret_cast<bool *>(data);
        *closed = true;
        throw retry_error("libvirt connection closed");
    };
    auto free_func = [](void *) {
    };
    lv.virConnectRegisterCloseCallback(conn_, close_func, &closed_, free_func);

    libvirt::ref().virConnSetErrorFunc(conn_, nullptr, virt::on_libvirt_error);
    return *this;
}

std::string virt::connection::capabilities() const
{
    return libvirt::ref().virConnectGetCapabilities(conn_);
}

std::string virt::connection::hostname() const
{
    return libvirt::ref().virConnectGetHostname(conn_);
}

unsigned long virt::connection::library_version() const
{
    unsigned long version = 0;
    libvirt::ref().virConnectGetLibVersion(conn_, &version);
    return version;
}

int virt::connection::max_vcpus(const char *type) const
{
    return libvirt::ref().virConnectGetMaxVcpus(conn_, type);
}

std::string virt::connection::sysinfo() const
{
    return libvirt::ref().virConnectGetSysinfo(conn_, 0);
}

const char *virt::connection::type() const
{
    return libvirt::ref().virConnectGetType(conn_);
}

std::string virt::connection::uri() const
{
    return libvirt::ref().virConnectGetURI(conn_);
}

unsigned long virt::connection::version() const
{
    unsigned long version = 0;
    libvirt::ref().virConnectGetVersion(conn_, &version);
    return version;
}

bool virt::connection::encrypted() const
{
    return libvirt::ref().virConnectIsEncrypted(conn_);
}

bool virt::connection::secure() const
{
    return libvirt::ref().virConnectIsSecure(conn_);
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

domain_ptr virt::connection::get_domain_ptr(const std::string &name)
{
    auto &lv = libvirt::ref();
    auto domain = lv.virDomainLookupByName(conn_, name.c_str());
    if (!domain) {
        throw std::domain_error("virDomainLookupByName error");
    }
    return domain;
}

connect_ptr virt::connection::get_ptr()
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

void virt::on_libvirt_error(void *, webvirt::error_ err)
{
    static const std::list<std::string> whitelist {
        "Requested metadata element is not present",
    };
    for (auto &whitelisted : whitelist) {
        if (strstr(err->message, whitelisted.c_str()) != nullptr) {
            return;
        }
    }

    logger::error(err->message);
}
