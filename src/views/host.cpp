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
#include "../logging.hpp"
#include <iostream>
using namespace webvirt::views;

void host::show(virt::connection &conn, const std::smatch &location,
                const http::request &, http::response &response)
{
    Json::Value output(Json::objectValue);

    output["hostname"] = conn.hostname();
    output["libVersion"] = conn.library_version();
    output["uri"] = conn.uri();

    const std::string user(location[1]);
    if (user == "root") {
        auto sysinfo = conn.sysinfo();
        pugi::xml_document doc;
        doc.load_string(sysinfo.c_str());
        output["sysinfo"] = json::xml_to_json(doc.child("sysinfo"));
    }

    output["version"] = conn.version();
    output["encrypted"] = conn.encrypted();
    output["secure"] = conn.secure();

    const char *type = conn.type();
    output["type"] = type;
    output["max_vcpus"] = conn.max_vcpus(type);

    auto capabilities = conn.capabilities();
    pugi::xml_document doc;
    doc.load_string(capabilities.c_str());
    output["caps"] =
        json::xml_to_json(doc.child("capabilities").child("host"));

    return http::set_response(response, output, beast::http::status::ok);
}

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
