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
#include "../json.hpp"
#include "../virt/util.hpp"
#include <fmt/format.h>
#include <fstream>
#include <pugixml.hpp>
using namespace webvirt::views;

webvirt::virt::domain get_domain(webvirt::virt::connection &conn,
                                 const std::string &name,
                                 webvirt::http::response &response)
{
    try {
        return conn.domain(name);
    } catch (const std::domain_error &) {
        Json::Value error(Json::objectValue);
        error["detail"] = fmt::format("Domain '{}' not found", name);
        response.body().append(webvirt::json::stringify(error));
        response.content_length(response.body().size());
        response.result(boost::beast::http::status::not_found);
    }
    return webvirt::virt::domain();
}

void domains::index(virt::connection &conn, const std::string &,
                    const std::smatch &, const http::request &,
                    http::response &response)
{
    auto domains = conn.domains();

    Json::Value data(Json::arrayValue);
    for (auto &domain : domains) {
        Json::Value item(Json::objectValue);
        item["id"] = domain.id();
        item["name"] = domain.name();

        int state = domain.state();
        item["state"] = Json::Value(Json::objectValue);
        item["state"]["id"] = state;
        item["state"]["string"] = virt::state_string(state);

        pugi::xml_document doc = domain.xml_document();
        auto domain_ = doc.child("domain");
        item["title"] = domain_.child("title").text().as_string();

        data.append(std::move(item));
    }

    response.body().append(json::stringify(std::move(data)));
    response.content_length(response.body().size());
}

void domains::show(virt::connection &conn, const std::string &,
                   const std::smatch &location, const http::request &,
                   http::response &response)
{
    const std::string name(location[2]);
    auto domain = get_domain(conn, name, response);
    if (!domain)
        return;

    pugi::xml_document doc = domain.xml_document();
    auto domain_ = doc.child("domain");

    Json::Value data(Json::objectValue);
    data["name"] = name;
    data["autostart"] = domain.autostart();
    data["id"] = domain_.attribute("id").as_int();
    data["uuid"] = domain_.child("uuid").text().as_string();
    data["title"] = domain_.child("title").text().as_string();
    data["description"] = domain_.child("description").text().as_string();
    data["type"] = domain_.attribute("type").as_string();

    Json::Value info(Json::objectValue);
    info["cpus"] = domain_.child("vcpu").text().as_uint();
    info["maxMemory"] = domain_.child("memory").text().as_uint();
    info["memory"] = domain_.child("currentMemory").text().as_uint();
    info["os"] = Json::Value(Json::objectValue);
    info["os"]["type"] = Json::Value(Json::objectValue);
    auto os = domain_.child("os");
    info["os"]["type"]["arch"] =
        os.child("type").attribute("arch").as_string();
    info["os"]["type"]["machine"] =
        os.child("type").attribute("machine").as_string();
    info["os"]["boot"] = Json::Value(Json::objectValue);
    info["os"]["boot"]["dev"] = os.child("boot").attribute("dev").as_string();

    info["devices"] = Json::Value(Json::objectValue);
    info["devices"]["emulator"] =
        domain_.child("devices").child("emulator").text().as_string();
    info["devices"]["disks"] = Json::Value(Json::arrayValue);
    auto disks = domain_.child("devices").children("disk");
    for (auto &disk : disks) {
        Json::Value object(Json::objectValue);
        auto device_type = disk.attribute("device").as_string();
        object["device"] = device_type;
        object["driver"] = Json::Value(Json::objectValue);
        object["driver"]["name"] =
            disk.child("driver").attribute("name").as_string();
        object["driver"]["type"] =
            disk.child("driver").attribute("type").as_string();
        object["source"] = Json::Value(Json::objectValue);
        object["source"]["file"] =
            disk.child("source").attribute("file").as_string();
        object["target"] = Json::Value(Json::objectValue);
        auto device = disk.child("target").attribute("dev").as_string();
        object["target"]["dev"] = device;
        object["target"]["bus"] =
            disk.child("target").attribute("bus").as_string();

        static const std::string target_type = "disk";
        if (device_type == target_type) {
            auto block_info_ptr = domain.block_info(device);
            object["block_info"] = Json::Value(Json::objectValue);

            // Size values in Kilobytes
            object["block_info"]["unit"] = "KiB";
            object["block_info"]["capacity"] =
                static_cast<unsigned long>(block_info_ptr->capacity / 1000);
            object["block_info"]["allocation"] =
                static_cast<unsigned long>(block_info_ptr->allocation / 1000);
            object["block_info"]["physical"] =
                static_cast<unsigned long>(block_info_ptr->physical / 1000);
        }

        info["devices"]["disks"].append(std::move(object));
    }

    info["devices"]["interfaces"] = Json::Value(Json::arrayValue);
    auto interfaces = domain_.child("devices").children("interface");
    for (auto &interface : interfaces) {
        Json::Value object(Json::objectValue);
        object["macAddress"] =
            interface.child("mac").attribute("address").as_string();
        object["model"] =
            interface.child("model").attribute("type").as_string();
        object["name"] =
            interface.child("alias").attribute("name").as_string();
        info["devices"]["interfaces"].append(std::move(object));
    }

    data["info"] = std::move(info);

    int state = domain.state();
    data["state"] = Json::Value(Json::objectValue);
    data["state"]["id"] = state;
    data["state"]["string"] = virt::state_string(state);

    response.body().append(json::stringify(data));
    response.content_length(response.body().size());
}

void domains::autostart(virt::connection &conn, const std::string &,
                        const std::smatch &location,
                        const http::request &request, http::response &response)
{
    const std::string name(location[2]);
    auto domain = get_domain(conn, name, response);
    if (!domain)
        return;

    bool enabled = request.method() == beast::http::verb::post;
    domain.autostart(enabled);

    Json::Value object(Json::objectValue);
    object["autostart"] = enabled;

    response.body().append(json::stringify(std::move(object)));
    response.content_length(response.body().size());
}

void domains::metadata(virt::connection &conn, const std::string &,
                       const std::smatch &location,
                       const http::request &request, http::response &response)
{
    const std::string name(location[2]);

    auto domain = get_domain(conn, name, response);
    if (!domain)
        return;

    // JSON output from this function.
    Json::Value output(Json::objectValue);

    Json::Value data(Json::objectValue);
    try {
        data = json::parse(request.body());
    } catch (const std::invalid_argument &) {
        output["detail"] = "Invalid JSON input";
        response.result(beast::http::status::bad_request);
    }

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

    response.set(boost::beast::http::field::content_type, "application/json");
    response.body().append(json::stringify(output));
    response.content_length(response.body().size());
}

void domains::start(virt::connection &conn, const std::string &,
                    const std::smatch &location, const http::request &,
                    http::response &response)
{
    const std::string name(location[2]);
    auto domain = get_domain(conn, name, response);
    if (!domain)
        return;

    if (!domain.start()) {
        Json::Value data(Json::objectValue);
        data["detail"] = "Unable to start domain";
        response.result(beast::http::status::bad_request);
        response.body().append(json::stringify(data));
        response.content_length(response.body().size());
        return;
    }

    response.result(beast::http::status::created);
    response.body().append(json::stringify(domain.simple_json()));
    response.content_length(response.body().size());
}

void domains::shutdown(virt::connection &conn, const std::string &,
                       const std::smatch &location, const http::request &,
                       http::response &response)
{
    const std::string name(location[2]);
    auto domain = get_domain(conn, name, response);
    if (!domain)
        return;

    bool ok = false;
    try {
        ok = domain.shutdown();
    } catch (const std::out_of_range &exc) {
        Json::Value data(Json::objectValue);
        data["detail"] = "Shutdown operation timed out";
        response.result(beast::http::status::gateway_timeout);
        response.body().append(json::stringify(data));
        response.content_length(response.body().size());
        return;
    }

    if (!ok) {
        Json::Value data(Json::objectValue);
        data["detail"] = "Unable to shutdown domain";
        response.result(beast::http::status::bad_request);
        response.body().append(json::stringify(data));
        response.content_length(response.body().size());
        return;
    }

    response.body().append(json::stringify(domain.simple_json()));
    response.content_length(response.body().size());
}
