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
#include "host.hpp"
#include "../http/util.hpp"
#include "../json.hpp"
#include <iostream>
using namespace webvirt::views;

void host::networks(virt::connection &conn, const std::smatch &,
                    const http::request &, http::response &response)
{
    Json::Value output(Json::arrayValue);

    auto networks = conn.networks();
    for (auto &network : networks) {
        pugi::xml_document xml = network.xml_document();
        output.append(json::xml_to_json(xml.child("network")));
    }

    return http::set_response(response, output, beast::http::status::ok);
}
