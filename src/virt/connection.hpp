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
    struct free_conn {
        void operator()(virConnectPtr ptr);
    };

private:
    std::shared_ptr<virConnect> conn_ = nullptr;
    int errno_ = 0;

public:
    connection() = default;
    connection(const connection &conn);
    connection(const std::string &uri);

    connection &operator=(const connection &conn);

    connection &connect(const std::string &str);

    std::vector<std::map<std::string, Json::Value>> domains();
    Json::Value domain(const std::string &);
    std::string xml_desc(const std::string &domain);

    int error();
    const char *strerror();

private:
    std::string _xml_desc(libvirt::domain_ptr);
};

}; // namespace webvirt::virt

#endif /* VIRT_CONNECTION_HPP */
