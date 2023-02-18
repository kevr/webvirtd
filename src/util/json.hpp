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
#ifndef UTIL_JSON_HPP
#define UTIL_JSON_HPP

#include <boost/beast.hpp>
#include <json/json.h>
#include <pugixml.hpp>
#include <stdexcept>
#include <string>

namespace webvirt::json
{

Json::Value parse(const std::string &str);
Json::Value parse(const boost::beast::multi_buffer &);

Json::Value error(const std::string &detail);

std::string stringify(const Json::Value &json);

Json::Value xml_to_json(const pugi::xml_node &node);

}; // namespace webvirt::json

#endif /* UTIL_JSON_HPP */
