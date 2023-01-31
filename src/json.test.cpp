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
#include <gtest/gtest.h>
using namespace webvirt;

TEST(json, parse)
{
    auto object = json::parse("{\"key\": \"value\"}");
    EXPECT_EQ(object["key"].asString(), "value");
}

TEST(json, parse_error)
{
    EXPECT_THROW(json::parse("not-json"), std::invalid_argument);
}

TEST(json, parse_buffer)
{
    boost::beast::multi_buffer buffer;
    boost::beast::ostream(buffer) << "{\"key\": \"value\"}";
    auto object = json::parse(buffer);
    EXPECT_EQ(object["key"].asString(), "value");
}

TEST(json, stringify)
{
    Json::Value object(Json::objectValue);
    object["key"] = "value";
    EXPECT_EQ(json::stringify(object), "{\"key\":\"value\"}\n");
}
