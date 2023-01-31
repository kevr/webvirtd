/* SPDX-License-Identifier: Apache 2.0 */
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
