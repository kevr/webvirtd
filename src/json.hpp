/* SPDX-License-Identifier: Apache 2.0 */
#ifndef JSON_HPP
#define JSON_HPP

#include <boost/beast.hpp>
#include <json/json.h>
#include <stdexcept>
#include <string>

namespace webvirt::json
{

Json::Value parse(const std::string &str);
Json::Value parse(const boost::beast::multi_buffer &);

std::string stringify(const Json::Value &json);

}; // namespace webvirt::json

#endif /* JSON_HPP */
