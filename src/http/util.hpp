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
#ifndef HTTP_UTIL_HPP
#define HTTP_UTIL_HPP

#include <http/types.hpp>

#include <json/json.h>

namespace webvirt::http
{

void set_response(http::response &, const std::string &, beast::http::status);
void set_response(http::response &, const Json::Value &, beast::http::status);

}; // namespace webvirt::http

#endif /* HTTP_UTIL_HPP */
