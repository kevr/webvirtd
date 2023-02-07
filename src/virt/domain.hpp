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
#ifndef VIRT_DOMAIN_HPP
#define VIRT_DOMAIN_HPP

#include "../libvirt.hpp"
#include <json/json.h>
#include <pugixml.hpp>

namespace webvirt::virt
{

class domain
{
private:
    libvirt::domain_ptr domain_;

public:
    domain() = default;
    domain(libvirt::domain_ptr);
    domain(const domain &);
    domain(domain &&);
    domain &operator=(domain);

    operator bool() const;

    int id() const;
    std::string name() const;
    int state() const;
    bool autostart() const;
    void autostart(bool enabled);

    std::string xml_desc();
    pugi::xml_document xml_document();

    libvirt::block_info_ptr block_info(const std::string &);

    bool start();
    bool shutdown();

    Json::Value simple_json() const;
};

}; // namespace webvirt::virt

#endif /* VIRT_DOMAIN_HPP */
