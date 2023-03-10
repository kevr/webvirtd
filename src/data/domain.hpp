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
#ifndef DATA_DOMAIN_HPP
#define DATA_DOMAIN_HPP

#include <virt/domain.hpp>

#include <json/json.h>

namespace webvirt::data
{

/** Produce a simple JSON object for a libvirt domain
 *
 * See https://app.swaggerhub.com/apis/kevr/webvirtd for:
 * - GET /users/(user)/domains/
 *
 * @param domain libvirt domain
 * @returns JSON object for a libvirt domain
 **/
Json::Value simple_domain(virt::domain &);

/** Produces a more detailed JSON object for a libvirt domain
 *
 * See https://app.swaggerhub.com/apis/kevr/webvirtd for:
 * - GET /users/(user)/domain/(name)/
 *
 * @param domain libvirt domain
 * @returns Detailed JSON object for a libvirt domain
 **/
Json::Value domain(virt::domain &);

}; // namespace webvirt::data

#endif /* DATA_DOMAIN_HPP */
