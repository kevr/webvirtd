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
#include <cstring>
#include <errno.h>
#include <json/json.h>
#include <libvirt/libvirt.h>
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
    std::shared_ptr<virConnect> conn_ = nullptr;
    int errno_ = 0;

public:
    connection() = default;
    connection(const connection &conn);
    connection(const std::string &uri);

    connection &operator=(const connection &conn);
    bool operator==(const connection &other) const;
    bool operator!=(const connection &other) const;
    connection &connect(const std::string &str);

    Json::Value domains();
    Json::Value domain(const std::string &);

    libvirt::domain_ptr get_domain_ptr(const std::string &name);
    std::string xml_desc(const std::string &domain);
    std::string xml_desc(libvirt::domain_ptr);

    libvirt::block_info_ptr get_block_info_ptr(libvirt::domain_ptr,
                                               const std::string &device);

    bool start(libvirt::domain_ptr);
    bool shutdown(libvirt::domain_ptr);

    int error();
    const char *strerror();
};

}; // namespace webvirt::virt

#endif /* VIRT_CONNECTION_HPP */
