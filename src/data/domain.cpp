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
#include <data/domain.hpp>
#include <util/json.hpp>
#include <virt/util.hpp>

using namespace webvirt;

Json::Value data::simple_domain(virt::domain &domain)
{
    Json::Value output(Json::objectValue);

    output["id"] = domain.id();

    Json::Value name(Json::objectValue);
    name["text"] = domain.name();
    output["name"] = std::move(name);

    Json::Value title(Json::objectValue);
    title["text"] = domain.title();
    output["title"] = std::move(title);

    Json::Value description(Json::objectValue);
    description["text"] = domain.description();
    output["description"] = std::move(description);

    int state_id = domain.state();
    Json::Value attrib(Json::objectValue);
    attrib["id"] = state_id;
    attrib["string"] = virt::state_string(state_id);

    Json::Value state(Json::objectValue);
    state["attrib"] = std::move(attrib);
    output["state"] = std::move(state);

    return output;
}

Json::Value data::domain(virt::domain &domain)
{
    Json::Value output = simple_domain(domain);

    // Include metadata not included elsewhere
    output["autostart"] = domain.autostart();

    // Integrate the XML document into `output`.
    // 1. json::xml_to_json
    // 2. Copy each resulting JSON key/value pair into `output`
    pugi::xml_document doc = domain.xml_document();
    Json::Value xml_output = json::xml_to_json(doc.child("domain"));
    for (const std::string &key : xml_output.getMemberNames()) {
        output[key] = xml_output[key];
    }

    // Turn various JSON values into arrays
    std::vector<std::string> keys = { "interface", "disk" };
    for (const auto &key : keys) {
        if (output["devices"].isMember(key) &&
            output["devices"][key].type() == Json::objectValue) {
            auto current = output["devices"][key];
            output["devices"][key] = Json::Value(Json::arrayValue);
            output["devices"][key].append(std::move(current));
        }
    }

    // For each disk found which has a device == "disk", collect
    // and include block information.
    if (output["devices"].isMember("disk")) {
        for (auto &disk : output["devices"]["disk"]) {
            // If this is not a storage disk, continue on.
            if (disk["attrib"]["device"].asString() != "disk")
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

    return output;
}
