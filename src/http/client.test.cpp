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
#include "client.hpp"
#include "io_service.hpp"
#include <gtest/gtest.h>
using namespace webvirt;

TEST(client, connect_fails)
{
    http::io_service io;
    auto client = std::make_shared<http::unix_client>(io, "test.sock");
    beast::error_code ec;
    client->on_error([&](const char *, beast::error_code ec_) {
        ec = ec_;
    });
    client->async_get("/test/").run();

    EXPECT_EQ(ec.message(), "No such file or directory");
}
