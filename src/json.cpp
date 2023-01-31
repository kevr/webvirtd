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
using namespace webvirt;

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

std::string json::stringify(const Json::Value &json)
{
    Json::FastWriter writer;
    return writer.write(json);
}
