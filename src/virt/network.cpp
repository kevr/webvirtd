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
#include "network.hpp"
using namespace webvirt::virt;

std::string network::xml_desc()
{
    return libvirt::ref().virNetworkGetXMLDesc(ptr_, 0);
}

pugi::xml_document network::xml_document()
{
    pugi::xml_document doc;
    auto desc = xml_desc();
    doc.load_string(desc.c_str());
    return doc;
}
