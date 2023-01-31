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
#include "app.hpp"
#include "http/client.hpp"
#include "mocks/libvirt.hpp"
#include "util.hpp"
#include <gtest/gtest.h>
#include <json/json.h>
#include <thread>
using namespace webvirt;

using testing::_;
using testing::Invoke;
using testing::Return;
using testing::Test;

class app_test : public Test
{
protected:
    std::filesystem::path tmpdir, socket_path;

    webvirt::io_service io_;
    std::shared_ptr<webvirt::app> app_;
    std::thread server_thread;

    using client_t = http::client<net::unix>;
    webvirt::io_service client_io_;
    std::shared_ptr<client_t> client;

public:
    void SetUp() override
    {
        tmpdir = socket_path = webvirt::make_tmpdir();
        socket_path /= "socket.sock";
        app_ = std::make_shared<webvirt::app>(io_, socket_path);
        client = std::make_shared<client_t>(client_io_, socket_path);

        server_thread = std::thread([&] {
            app_->run();
        });
    }

    void TearDown() override
    {
        server_thread.join();
    }
};

class mock_app_test : public app_test
{
protected:
    char *mem_;
    libvirt::connect *ptr_ = nullptr;
    mocks::libvirt lv;

    uid_t uid;
    std::string username;

public:
    void SetUp() override
    {
        app_test::SetUp();
        libvirt::change(lv);

        mem_ = new char;
        ptr_ = reinterpret_cast<libvirt::connect *>(mem_);

        auto &sys = syscaller::instance();
        uid = sys.getuid();
        auto *passwd = sys.getpwuid(uid);
        username = passwd->pw_name;
    }

    void TearDown() override
    {
        app_test::TearDown();
        libvirt::reset();
    }
};

TEST_F(app_test, method_not_allowed)
{
    http::response response;
    client->on_response([&response, this](const auto &response_) {
        response = response_;
        io_.stop();
    });
    client->async_options("/domains/").run();

    EXPECT_EQ(response.result_int(),
              static_cast<int>(beast::http::status::method_not_allowed));
}

TEST_F(app_test, not_found)
{
    http::response response;
    client->on_response([&response, this](const auto &response_) {
        response = response_;
        io_.stop();
    });
    client->async_get("/not-found/").run();

    EXPECT_EQ(response.result_int(),
              static_cast<int>(beast::http::status::not_found));
}

TEST_F(app_test, append_trailing_slash)
{
    http::response response;
    client->on_response([&response, this](const auto &response_) {
        response = response_;
        io_.stop();
    });
    client->async_get("/blah").run();
    EXPECT_EQ(response.result_int(),
              static_cast<int>(beast::http::status::temporary_redirect));
    EXPECT_EQ(response.at(beast::http::field::location), "/blah/");
}
