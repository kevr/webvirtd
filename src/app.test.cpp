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
#include "config.hpp"
#include "http/client.hpp"
#include "mocks/libvirt.hpp"
#include "retry.hpp"
#include "util.hpp"
#include "json/forwards.h"
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <json/json.h>
#include <thread>
#include <tuple>
using namespace webvirt;
using namespace std::string_literals;

using testing::_;
using testing::Invoke;
using testing::Return;
using testing::Test;

class app_test : public Test
{
protected:
    std::filesystem::path tmpdir, socket_path;

    http::io_service io_;
    std::shared_ptr<webvirt::app> app_;
    std::thread server_thread;

    using client_t = http::client<net::unix>;
    http::io_service client_io_;
    std::shared_ptr<client_t> client;

    http::response response;

public:
    void SetUp() override
    {
        tmpdir = socket_path = webvirt::make_tmpdir();
        socket_path /= "socket.sock";
        app_ = std::make_shared<webvirt::app>(io_, socket_path);
        client = std::make_shared<client_t>(client_io_, socket_path);

        client->on_response([this](const auto &response_) {
            response = response_;
            io_.stop();
        });

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
    libvirt::connect_ptr conn;
    mocks::libvirt lv;

    uid_t uid;
    std::string username;

    // libvirt control
    bool libvirt_closed = false;
    std::function<void(webvirt::connect *, int, void *)> libvirt_close;
    std::function<void(void *)> libvirt_free;

public:
    void SetUp() override
    {
        app_test::SetUp();
        libvirt::change(lv);

        conn = std::make_shared<webvirt::connect>();

        auto &sys = syscaller::ref();
        uid = sys.getuid();
        auto *passwd = sys.getpwuid(uid);
        username = passwd->pw_name;

        auto &conf = config::ref();
        conf.add_option("libvirt-shutdown-timeout",
                        boost::program_options::value<double>()
                            ->default_value(0.01)
                            ->multitoken(),
                        "libvirt shutdown timeout");
        conf.add_option("libvirt-shutoff-timeout",
                        boost::program_options::value<double>()
                            ->default_value(0.02)
                            ->multitoken(),
                        "libvirt shutoff timeout");

        const char *argv[] = { "webvirtd" };
        conf.parse(1, argv);

        EXPECT_CALL(lv, virConnectRegisterCloseCallback(_, _, _, _))
            .WillRepeatedly(
                Invoke([&](libvirt::connect_ptr,
                           void (*close_fn)(webvirt::connect *, int, void *),
                           void *closed_ptr,
                           void (*free_fn)(void *)) {
                    libvirt_closed = *reinterpret_cast<bool *>(closed_ptr);
                    libvirt_close = close_fn;
                    libvirt_free = free_fn;
                    return 0;
                }));

        logger::enable_debug(true);
    }

    void TearDown() override
    {
        logger::reset_debug();
        app_test::TearDown();
        libvirt::reset();
    }
};

TEST_F(app_test, method_not_allowed)
{
    const char *endpoint = "/users/test/domains/";
    client->async_post(endpoint).run();

    EXPECT_EQ(response.result(), beast::http::status::method_not_allowed);
}

TEST_F(app_test, options)
{
    const char *endpoint = "/users/test/domains/";
    client->async_options(endpoint).run();

    EXPECT_EQ(response.at(beast::http::field::allow), "GET, OPTIONS");
}

TEST_F(app_test, options_not_found)
{
    const char *endpoint = "/users/test/blahblah/";
    client->async_options(endpoint).run();

    EXPECT_EQ(response.result(), beast::http::status::not_found);
}

TEST_F(app_test, not_found)
{
    client->async_get("/not-found/").run();

    EXPECT_EQ(response.result(), beast::http::status::not_found);
}

TEST_F(app_test, append_trailing_slash)
{
    client->async_get("/blah").run();
    EXPECT_EQ(response.result(), beast::http::status::temporary_redirect);
    EXPECT_EQ(response.at(beast::http::field::location), "/blah/");
}

TEST_F(mock_app_test, domains_libvirt_error)
{
    EXPECT_CALL(lv, virConnectOpen(_)).WillOnce(Return(nullptr));

    auto endpoint = fmt::format("/users/{}/domains/test/", username);
    client->async_get(endpoint.c_str()).run();

    EXPECT_EQ(response.result(), beast::http::status::internal_server_error);

    auto object = json::parse(response.body());
    EXPECT_TRUE(object.isObject());
    EXPECT_EQ(object["detail"], "Unable to connect to libvirt");
}

TEST_F(mock_app_test, domain_not_found)
{
    EXPECT_CALL(lv, virConnectOpen(_)).WillOnce(Return(conn));
    EXPECT_CALL(lv, virDomainLookupByName(_, _)).WillOnce(Return(nullptr));

    auto endpoint = fmt::format("/users/{}/domains/test/", username);
    client->async_get(endpoint.c_str()).run();

    EXPECT_EQ(response.result(), beast::http::status::not_found);

    // When routes are matched but not_found is returned, there should
    // be an Allow header present with configured methods.
    EXPECT_EQ(response.at(beast::http::field::allow), "GET, OPTIONS");
}

TEST_F(mock_app_test, persistent_virt_connection)
{
    EXPECT_CALL(lv, virConnectOpen(_)).Times(2).WillRepeatedly(Return(conn));

    libvirt::domain_ptr dom = std::make_shared<webvirt::domain>();
    EXPECT_CALL(lv, virDomainLookupByName(_, _))
        .Times(2)
        .WillRepeatedly(Return(dom));
    EXPECT_CALL(lv, virDomainGetAutostart(_, _))
        .Times(2)
        .WillRepeatedly(Invoke([](auto, int *autostart) {
            *autostart = 0;
            return 0;
        }));

    EXPECT_CALL(lv, virDomainGetID(_)).Times(2).WillRepeatedly(Return(1));
    EXPECT_CALL(lv, virDomainGetName(_))
        .Times(2)
        .WillRepeatedly(Return("test"));
    EXPECT_CALL(lv, virDomainGetState(_, _, _, _))
        .Times(2)
        .WillRepeatedly(Invoke([](auto, int *state, int *, int) {
            *state = VIR_DOMAIN_RUNNING;
            return 0;
        }));

    EXPECT_CALL(lv, virDomainGetMetadata(_, _, _, _)).Times(4);
    EXPECT_CALL(lv, virDomainGetXMLDesc(_, _)).Times(2);

    client->on_response([this](const auto &response_) {
        response = response_;
    });

    testing::internal::CaptureStdout();

    auto endpoint = fmt::format("/users/{}/domains/test/", username);
    client->async_get(endpoint.c_str()).run();
    EXPECT_EQ(response.result(), beast::http::status::ok);

    auto &connection = app_->pool().get(username);
    EXPECT_THROW(libvirt_close(nullptr, 0, &connection.closed()),
                 webvirt::retry_error);
    EXPECT_TRUE(connection.closed());

    EXPECT_NO_THROW(libvirt_free(nullptr));

    client->close();
    client->on_response([this](const auto &response_) {
        response = response_;
        io_.stop();
    });

    // Run get again after we've closed the libvirt connection.
    // During this run, reconnection will be made.
    client->async_get(endpoint.c_str()).run();
    EXPECT_EQ(response.result(), beast::http::status::ok);

    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("Reconnected to libvirt"), std::string::npos);
}
