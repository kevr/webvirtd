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
#ifndef DATA_HOST_HPP
#define DATA_HOST_HPP

#include <virt/connection.hpp>

#include <json/json.h>
#include <string>

namespace webvirt::data
{

/** Produce JSON data for a libvirt host
 *
 * See https://app.swaggerhub.com/apis/kevr/webvirtd for:
 * - GET /users/(user)/host/
 *
 * @param conn libvirt connection
 * @returns JSON object for the libvirt host
 **/
Json::Value host(virt::connection &);

/** Produce JSON data for a libvirt host's networks
 *
 * See https://app.swaggerhub.com/apis/kevr/webvirtd for:
 * - GET /users/(user)/host/networks/
 *
 * @param conn libvirt connection
 * @returns JSON array of network data for the libvirt host
 **/
Json::Value networks(virt::connection &);

}; // namespace webvirt::data

#endif /* DATA_HOST_HPP */
