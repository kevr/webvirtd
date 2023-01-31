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
#include "json/forwards.h"
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <json/json.h>
#include <thread>
#include <tuple>
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

public:
    void SetUp() override
    {
        app_test::SetUp();
        libvirt::change(lv);

        conn = std::make_shared<libvirt::connect>();

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

    std::string libvirt_domain_xml(
        unsigned id = 1, unsigned int vcpu = 2,
        unsigned int current_memory = 1024, unsigned int memory = 1024,
        std::vector<std::tuple<std::string, std::string, std::string>>
            interfaces = {})
    {
        std::string interfaces_;
        for (auto [mac, model, name] : interfaces) {
            interfaces_.append(fmt::format(R"(
<interface>
    <mac address="{}" />
    <model type="{}" />
    <alias name="{}" />
</interface>
)",
                                           mac,
                                           model,
                                           name));
        }

        return fmt::format(R"(
<domain id="{}">
    <vcpu>{}</vcpu>
    <currentMemory>{}</currentMemory>
    <memory>{}</memory>
    <devices>
        {}
    </devices>
</domain>
)",
                           id,
                           vcpu,
                           current_memory,
                           memory,
                           interfaces_);
    }
};

TEST_F(app_test, method_not_allowed)
{
    client->async_options("/domains/").run();

    EXPECT_EQ(response.result_int(),
              static_cast<int>(beast::http::status::method_not_allowed));
}

TEST_F(app_test, not_found)
{
    client->async_get("/not-found/").run();

    EXPECT_EQ(response.result_int(),
              static_cast<int>(beast::http::status::not_found));
}

TEST_F(app_test, append_trailing_slash)
{
    client->async_get("/blah").run();
    EXPECT_EQ(response.result_int(),
              static_cast<int>(beast::http::status::temporary_redirect));
    EXPECT_EQ(response.at(beast::http::field::location), "/blah/");
}

TEST_F(mock_app_test, domains)
{

    EXPECT_CALL(lv, virConnectOpen(_)).WillOnce(Return(conn));

    std::vector<libvirt::domain_ptr> domains;
    libvirt::domain_ptr dom = std::make_shared<libvirt::domain>();
    domains.emplace_back(dom);
    EXPECT_CALL(lv, virConnectListAllDomains(_, _)).WillOnce(Return(domains));

    const char *domain_name = "test-domain";
    EXPECT_CALL(lv, virDomainGetName(_)).WillOnce(Return(domain_name));
    EXPECT_CALL(lv, virDomainGetState(_, _, _, _))
        .WillOnce(Invoke([](auto, int *state, int *reason, int) {
            *state = VIR_DOMAIN_RUNNING;
            *reason = 0;
            return 0;
        }));
    EXPECT_CALL(lv, virDomainGetID(_)).WillOnce(Return(1));

    Json::Value data(Json::objectValue);
    data["user"] = username;

    client->async_post("/domains/", json::stringify(data)).run();

    auto array = json::parse(response.body());
    EXPECT_TRUE(array.isArray());
    EXPECT_EQ(array.size(), 1);

    auto object =
        array.get(Json::ArrayIndex(0), Json::Value(Json::objectValue));
    EXPECT_EQ(object["id"], 1);
    EXPECT_EQ(object["name"], "test-domain");
    EXPECT_EQ(object["state"]["id"], VIR_DOMAIN_RUNNING);
    EXPECT_EQ(object["state"]["string"], "Running");
}

TEST_F(mock_app_test, domains_libvirt_error)
{
    EXPECT_CALL(lv, virConnectOpen(_)).WillOnce(Return(nullptr));

    Json::Value data(Json::objectValue);
    data["user"] = username;
    client->async_post("/domains/", json::stringify(data)).run();

    EXPECT_EQ(response.result_int(),
              static_cast<int>(beast::http::status::internal_server_error));

    auto object = json::parse(response.body());
    EXPECT_TRUE(object.isObject());
    EXPECT_EQ(object["detail"], "Unable to connect to libvirt");
}

TEST_F(mock_app_test, domain)
{
    EXPECT_CALL(lv, virConnectOpen(_)).WillOnce(Return(conn));

    libvirt::domain_ptr dom = std::make_shared<libvirt::domain>();
    EXPECT_CALL(lv, virDomainLookupByName(_, _)).WillOnce(Return(dom));

    EXPECT_CALL(lv, virDomainGetState(_, _, _, _))
        .WillOnce(Invoke([](auto, int *state, int *, int) {
            *state = VIR_DOMAIN_RUNNING;
            return 0;
        }));

    auto buffer = libvirt_domain_xml();
    EXPECT_CALL(lv, virDomainGetXMLDesc(_, _)).WillOnce(Return(buffer));

    Json::Value data(Json::objectValue);
    data["user"] = username;
    client->async_post("/domains/test/", json::stringify(data)).run();

    EXPECT_EQ(response.result_int(),
              static_cast<int>(beast::http::status::ok));
    data = json::parse(response.body());
    EXPECT_EQ(data["id"], 1);
}

TEST_F(mock_app_test, domain_libvirt_error)
{
    EXPECT_CALL(lv, virConnectOpen(_)).WillOnce(Return(nullptr));

    Json::Value data(Json::objectValue);
    data["user"] = username;
    client->async_post("/domains/test/", json::stringify(data)).run();

    EXPECT_EQ(response.result_int(),
              static_cast<int>(beast::http::status::internal_server_error));

    auto object = json::parse(response.body());
    EXPECT_TRUE(object.isObject());
    EXPECT_EQ(object["detail"], "Unable to connect to libvirt");
}

TEST_F(mock_app_test, domain_unknown)
{
    EXPECT_CALL(lv, virConnectOpen(_)).WillOnce(Return(conn));
    EXPECT_CALL(lv, virDomainLookupByName(_, _)).WillOnce(Return(nullptr));

    Json::Value data(Json::objectValue);
    data["user"] = username;
    client->async_post("/domains/test/", json::stringify(data)).run();

    EXPECT_EQ(response.result_int(),
              static_cast<int>(beast::http::status::not_found));
}

TEST_F(mock_app_test, domain_interfaces)
{
    EXPECT_CALL(lv, virConnectOpen(_)).WillOnce(Return(conn));

    libvirt::domain_ptr dom = std::make_shared<libvirt::domain>();
    EXPECT_CALL(lv, virDomainLookupByName(_, _)).WillOnce(Return(dom));

    auto buffer = libvirt_domain_xml(
        1,
        2,
        1024,
        1024,
        { std::make_tuple<std::string, std::string, std::string>(
            "aa:bb:cc:dd:11:22:33:44", "virtio", "net0") });
    EXPECT_CALL(lv, virDomainGetXMLDesc(_, _)).WillOnce(Return(buffer));

    Json::Value data(Json::objectValue);
    data["user"] = username;
    client->async_post("/domains/test/interfaces/", json::stringify(data))
        .run();

    EXPECT_EQ(response.result_int(),
              static_cast<int>(beast::http::status::ok));

    data = json::parse(response.body());
    auto interface =
        data.get(Json::ArrayIndex(0), Json::Value(Json::objectValue));
    EXPECT_EQ(interface["macAddress"], "aa:bb:cc:dd:11:22:33:44");
    EXPECT_EQ(interface["model"], "virtio");
    EXPECT_EQ(interface["name"], "net0");
}

TEST_F(mock_app_test, domain_interfaces_libvirt_error)
{
    EXPECT_CALL(lv, virConnectOpen(_)).WillOnce(Return(nullptr));

    Json::Value data(Json::objectValue);
    data["user"] = username;
    client->async_post("/domains/test/interfaces/", json::stringify(data))
        .run();

    EXPECT_EQ(response.result_int(),
              static_cast<int>(beast::http::status::internal_server_error));

    auto object = json::parse(response.body());
    EXPECT_TRUE(object.isObject());
    EXPECT_EQ(object["detail"], "Unable to connect to libvirt");
}
