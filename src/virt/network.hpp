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
#ifndef VIRT_NETWORK_HPP
#define VIRT_NETWORK_HPP

#include "../libvirt.hpp"
#include "ptr_type.hpp"
#include <pugixml.hpp>

namespace webvirt::virt
{

class network : public ptr_type<network_ptr>
{
public:
    using ptr_type::ptr_type;

    std::string xml_desc();
    pugi::xml_document xml_document();
};

}; // namespace webvirt::virt

#endif /* VIRT_NETWORK_HPP */
