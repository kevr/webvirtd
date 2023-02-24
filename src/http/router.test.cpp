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
#include <http/middleware.hpp>
#include <http/router.hpp>
#include <mocks/syscall.hpp>
#include <util/json.hpp>
#include <util/retry.hpp>

#include <gtest/gtest.h>

using namespace webvirt;

using testing::_;
using testing::Return;
using testing::Test;

class router_test : public Test
{
protected:
    static void noop(http::connection_ptr, const std::smatch &,
                     const http::request &, http::response &)
    {
    }

    http::router router_;

    http::io_context io_;
    http::connection_ptr conn_;

public:
    void SetUp() override
    {
        conn_ = std::make_shared<http::connection>(
            io_, net::unix::socket { io_ }, std::chrono::milliseconds(50));
    }
};

TEST_F(router_test, noop)
{
    http::response response;
    noop(conn_, std::smatch(), http::request(), response);
}

TEST_F(router_test, with_user_invalid_user)
{
    mocks::syscall sys;
    syscall::change(sys);
    EXPECT_CALL(sys, getpwnam(_)).WillOnce(Return(nullptr));

    // with_user depends on std::smatch index [1].
    router_.route("^(.*)$", http::middleware::with_user(noop));

    http::request request;
    request.target("test");

    http::response response;
    router_.run(conn_, request, response);

    EXPECT_EQ(response.result_int(),
              static_cast<int>(beast::http::status::not_found));
    auto data = json::parse(response.body());
    EXPECT_EQ(data["detail"], "Unable to locate user");

    syscall::reset();
}

TEST_F(router_test, with_user_unknown_user)
{
    router_.route("/test/", http::middleware::with_user(noop));

    http::request request;
    request.target("/test/");

    Json::Value json(Json::objectValue);
    json["user"] = "test";
    beast::ostream(request.body()) << json::stringify(json);

    mocks::syscall sys;
    EXPECT_CALL(sys, getpwnam(_)).WillOnce(Return(nullptr));
    syscall::change(sys);

    http::response response;
    router_.run(conn_, request, response);

    EXPECT_EQ(response.result_int(),
              static_cast<int>(beast::http::status::not_found));
    auto data = json::parse(response.body());
    EXPECT_EQ(data["detail"], "Unable to locate user");
}

TEST_F(router_test, retry_until_fatal)
{
    router_.route(R"(^/retry/$)", [](auto, auto &, const auto &, auto &) {
        throw webvirt::retry_error("Retry!");
    });

    http::request request;
    request.target("/retry/");
    http::response response;
    router_.run(conn_, request, response);

    EXPECT_EQ(response.result(), beast::http::status::internal_server_error);
}
