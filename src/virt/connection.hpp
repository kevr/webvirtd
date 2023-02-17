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
#ifndef VIRT_CONNECTION_HPP
#define VIRT_CONNECTION_HPP

#include "../libvirt.hpp"
#include "../logging.hpp"
#include "domain.hpp"
#include "network.hpp"
#include <atomic>
#include <cstring>
#include <errno.h>
#include <json/json.h>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace webvirt::virt
{

class connection
{
private:
    libvirt::connect_ptr conn_ { nullptr };
    int errno_ { 0 };
    bool closed_ { true };

public:
#ifdef TEST_BUILD
    bool &closed();
#else
    bool closed();
#endif

public:
    connection() = default;
    connection(const connection &conn);
    connection(const std::string &uri);

    connection &operator=(const connection &conn);
    operator bool();
    bool operator==(const connection &other) const;
    bool operator!=(const connection &other) const;

    connection &connect(const std::string &str);

    std::string capabilities() const;
    std::string hostname() const;
    unsigned long library_version() const;
    int max_vcpus(const char *) const;
    std::string sysinfo() const;
    const char *type() const;
    std::string uri() const;
    unsigned long version() const;
    bool encrypted() const;
    bool secure() const;

    std::vector<virt::domain> domains();
    virt::domain domain(const std::string &name);
    libvirt::domain_ptr get_domain_ptr(const std::string &name);

    std::vector<virt::network> networks();

    libvirt::connect_ptr get_ptr();

    int error();
    const char *strerror();
};

void on_libvirt_error(void *, webvirt::error_);
}; // namespace webvirt::virt

#endif /* VIRT_CONNECTION_HPP */
