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
#include "router.hpp"
#include "../json.hpp"
#include "../mocks/syscaller.hpp"
#include <gtest/gtest.h>
using namespace webvirt;

using testing::_;
using testing::Return;
using testing::Test;

class router_test : public Test
{
protected:
    static void noop(const std::string &, const std::smatch &,
                     const http::request &, http::response &)
    {
    }

    http::router router_;
};

TEST_F(router_test, noop)
{
    http::response response;
    noop("", std::smatch(), http::request(), response);
}

TEST_F(router_test, with_user_invalid_json)
{
    router_.route("/test/", http::router::with_user(noop));

    http::request request;
    request.target("/test/");

    http::response response;
    router_.run(request, response);

    EXPECT_EQ(response.result_int(),
              static_cast<int>(beast::http::status::bad_request));
    auto data = json::parse(response.body());
    EXPECT_EQ(data["detail"], "Unable to parse request body JSON");
}

TEST_F(router_test, with_user_unknown_user)
{
    router_.route("/test/", http::router::with_user(noop));

    http::request request;
    request.target("/test/");

    Json::Value json(Json::objectValue);
    json["user"] = "test";
    beast::ostream(request.body()) << json::stringify(json);

    mocks::syscaller sys;
    EXPECT_CALL(sys, getpwnam(_)).WillOnce(Return(nullptr));
    syscaller::change(&sys);

    http::response response;
    router_.run(request, response);

    EXPECT_EQ(response.result_int(),
              static_cast<int>(beast::http::status::not_found));
    auto data = json::parse(response.body());
    EXPECT_EQ(data["detail"], "Unable to locate user");
}