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
#include "domains.hpp"
#include "../http/util.hpp"
#include "../util/json.hpp"
#include "../virt/util.hpp"
#include <boost/cast.hpp>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <pugixml.hpp>
using namespace webvirt::views;
using namespace std::string_literals;

void domains::index(virt::connection &conn, const std::smatch &,
                    const http::request &, http::response &response)
{
    auto domains = conn.domains();

    Json::Value data(Json::arrayValue);
    for (auto &domain : domains) {
        data.append(domain.simple_json());
    }

    return http::set_response(response, data, beast::http::status::ok);
}

void domains::show(virt::connection &, virt::domain domain,
                   const std::smatch &location, const http::request &,
                   http::response &response)
{
    const std::string name(location[2]);

    auto data = domain.simple_json();
    auto doc = domain.xml_document();

    auto xml_data = json::xml_to_json(doc.child("domain"));
    for (const auto &key : xml_data.getMemberNames()) {
        data[key] = xml_data[key];
    }

    data["autostart"] = domain.autostart();

    std::vector<std::string> keys = { "interface", "disk" };
    for (const auto &key : keys) {
        if (data["devices"][key] &&
            data["devices"][key].type() == Json::objectValue) {
            auto current = data["devices"][key];
            data["devices"][key] = Json::Value(Json::arrayValue);
            data["devices"][key].append(std::move(current));
        }
    }

    if (data["devices"]["disk"]) {
        for (auto &disk : data["devices"]["disk"]) {
            // If this is not a storage disk, continue on.
            if (disk["attrib"]["device"].asString() != "disk"s)
                continue;

            // Collect block_info sizing for the disk.
            auto dev = disk["target"]["attrib"]["dev"].asString();
            auto block_info_ptr = domain.block_info(dev);
            auto block_info = Json::Value(Json::objectValue);
            block_info["unit"] = "KiB";
            block_info["capacity"] = boost::numeric_cast<unsigned long>(
                block_info_ptr->capacity / 1000);
            block_info["allocation"] = boost::numeric_cast<unsigned long>(
                block_info_ptr->allocation / 1000);
            block_info["physical"] = boost::numeric_cast<unsigned long>(
                block_info_ptr->physical / 1000);
            disk["block_info"] = std::move(block_info);
        }
    }

    return http::set_response(response, data, beast::http::status::ok);
}

void domains::autostart(virt::connection &, virt::domain domain,
                        const std::smatch &location,
                        const http::request &request, http::response &response)
{
    const std::string name(location[2]);

    bool enabled = request.method() == beast::http::verb::post;
    domain.autostart(enabled);

    Json::Value object(Json::objectValue);
    object["autostart"] = enabled;

    return http::set_response(response, object, beast::http::status::ok);
}

void domains::bootmenu(virt::connection &conn, virt::domain domain,
                       const std::smatch &location,
                       const http::request &request, http::response &response)
{
    const std::string name(location[2]);

    auto doc = domain.xml_document();
    auto os = doc.child("domain").child("os");
    std::string enabled =
        request.method() == beast::http::verb::post ? "yes" : "no";
    os.remove_child("bootmenu");
    os.append_child("bootmenu");
    os.last_child().append_attribute("enable");
    os.last_child().last_attribute().set_value(enabled.c_str());

    std::stringstream ss;
    doc.save(ss);
    auto xml = ss.str();

    if (!domain.define_xml(conn.get_ptr(), xml.c_str())) {
        return http::set_response(response,
                                  json::error("Unable to replace domain XML"),
                                  beast::http::status::internal_server_error);
    }

    Json::Value data(Json::objectValue);
    data["type"] = json::xml_to_json(os.child("type"));
    data["boot"] = json::xml_to_json(os.child("boot"));
    data["bootmenu"] = json::xml_to_json(os.child("bootmenu"));

    return http::set_response(response, data, beast::http::status::ok);
}

void domains::metadata(virt::connection &, virt::domain domain,
                       const std::smatch &location,
                       const http::request &request, http::response &response)
{
    const std::string name(location[2]);

    Json::Value data(Json::objectValue);
    try {
        data = json::parse(request.body());
    } catch (const std::invalid_argument &) {
        return http::set_response(response,
                                  json::error("Invalid JSON input"),
                                  beast::http::status::bad_request);
    }

    // JSON output from this function.
    Json::Value output(Json::objectValue);

    auto current_title =
        domain.metadata(VIR_DOMAIN_METADATA_TITLE, nullptr, 0);
    auto title = data.get("title", current_title).asString();
    if (title != current_title) {
        domain.metadata(
            VIR_DOMAIN_METADATA_TITLE, title.c_str(), nullptr, nullptr, 0);
        output["title"] = title;
    }

    auto current_desc =
        domain.metadata(VIR_DOMAIN_METADATA_DESCRIPTION, nullptr, 0);
    auto desc = data.get("description", current_desc).asString();
    if (desc != current_desc) {
        domain.metadata(VIR_DOMAIN_METADATA_DESCRIPTION,
                        desc.c_str(),
                        nullptr,
                        nullptr,
                        0);
        output["description"] = desc;
    }

    if (!output.size()) {
        response.result(beast::http::status::not_modified);
    }

    return http::set_response(response, output, response.result());
}

void domains::start(virt::connection &, virt::domain domain,
                    const std::smatch &location, const http::request &,
                    http::response &response)
{
    const std::string name(location[2]);

    if (!domain.start()) {
        return http::set_response(response,
                                  json::error("Unable to start domain"),
                                  beast::http::status::bad_request);
    }

    return http::set_response(
        response, domain.simple_json(), beast::http::status::created);
}

void domains::shutdown(virt::connection &, virt::domain domain,
                       const std::smatch &location, const http::request &,
                       http::response &response)
{
    const std::string name(location[2]);

    bool ok = false;
    try {
        ok = domain.shutdown();
    } catch (const std::out_of_range &exc) {
        return http::set_response(response,
                                  json::error("Shutdown operation timed out"),
                                  beast::http::status::gateway_timeout);
    }

    if (!ok) {
        return http::set_response(response,
                                  json::error("Unable to shutdown domain"),
                                  beast::http::status::bad_request);
    }

    return http::set_response(
        response, domain.simple_json(), beast::http::status::ok);
}
