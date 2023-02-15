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
#include "json.hpp"
#include <iostream>
using namespace webvirt;
using namespace std::string_literals;

Json::Value json::parse(const std::string &str)
{
    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(str.c_str(), root)) {
        std::string message("json error:\n");
        message.append(reader.getFormattedErrorMessages());
        message.push_back('\n');
        throw std::invalid_argument(message);
    }
    return root;
}

Json::Value json::parse(const boost::beast::multi_buffer &buffer)
{
    std::string content = boost::beast::buffers_to_string(buffer.cdata());
    return parse(content);
}

Json::Value json::error(const std::string &detail)
{
    Json::Value data(Json::objectValue);
    data["detail"] = detail;
    return data;
} // LCOV_EXCL_LINE

std::string json::stringify(const Json::Value &json)
{
    Json::FastWriter writer;
    return writer.write(json);
}

Json::Value json::xml_to_json(const pugi::xml_node &node)
{
    Json::Value data(Json::objectValue);

    Json::Value attributes(Json::objectValue);
    for (auto attr : node.attributes()) {
        attributes[attr.name()] = attr.value();
    }

    if (attributes.size()) {
        data["attrib"] = std::move(attributes);
    }

    auto text = std::string(node.text().as_string());
    if (text.size()) {
        data["text"] = text;
    }

    for (auto child : node.children()) {
        auto name = child.name();

        if (name == ""s)
            continue;

        if (data.isMember(name)) {
            if (data[name].type() != Json::arrayValue) {
                auto current = data[name];
                data[name] = Json::Value(Json::arrayValue);
                data[name].append(current);
            }
            data[name].append(xml_to_json(child));
        } else {
            data[name] = xml_to_json(child);
        }
    }

    return data;
}
