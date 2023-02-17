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
#include "host.hpp"
#include "../mocks/libvirt.hpp"
#include "../util/json.hpp"
#include "json/forwards.h"
#include <gtest/gtest.h>
#include <regex>
using namespace std::string_literals;
using namespace webvirt;

using testing::_;
using testing::Return;
using testing::Test;

class host_test : public Test
{
protected:
    mocks::libvirt lv;

    virt::connection conn_;
    http::request request_;
    http::response response_;

    views::host views_;

public:
    void SetUp() override
    {
        libvirt::change(lv);
        libvirt::connect_ptr ptr = std::make_shared<webvirt::connect>();
        EXPECT_CALL(lv, virConnectOpen(_)).WillOnce(Return(ptr));
        conn_.connect("socket.sock");

        request_.method(beast::http::verb::get);
    }

    void TearDown() override
    {
        libvirt::reset();
    }

protected:
    std::smatch make_location(const std::string &expr, const std::string &uri)
    {
        std::regex re(expr);
        std::smatch match;
        EXPECT_TRUE(std::regex_match(uri, match, re));
        return match;
    }
};

TEST_F(host_test, show)
{
    EXPECT_CALL(lv, virConnectGetCapabilities(_)).Times(2);
    EXPECT_CALL(lv, virConnectGetHostname(_)).WillRepeatedly(Return("test"));
    EXPECT_CALL(lv, virConnectGetLibVersion(_, _)).Times(2);
    EXPECT_CALL(lv, virConnectGetMaxVcpus(_, _)).WillRepeatedly(Return(2));
    EXPECT_CALL(lv, virConnectGetType(_)).WillRepeatedly(Return("QEMU"));
    EXPECT_CALL(lv, virConnectGetURI(_))
        .WillOnce(Return("qemu+ssh://test@localhost/session"))
        .WillOnce(Return("qemu+ssh://root@localhost/system"));
    EXPECT_CALL(lv, virConnectGetVersion(_, _)).Times(2);
    EXPECT_CALL(lv, virConnectIsEncrypted(_)).Times(2);
    EXPECT_CALL(lv, virConnectIsSecure(_)).WillRepeatedly(Return(1));

    // Make a request as test user.
    std::string endpoint("/users/test/info/");
    auto location = make_location(R"(^/users/([^/]+)/info/)", endpoint);
    views_.show(conn_, location, request_, response_);

    auto data = json::parse(response_.body());
    EXPECT_EQ(data["hostname"].asString(), "test");
    EXPECT_EQ(data["libVersion"].asUInt(), 0);
    EXPECT_EQ(data["uri"].asString(), "qemu+ssh://test@localhost/session");
    EXPECT_EQ(data["version"].asUInt(), 0);
    EXPECT_EQ(data["encrypted"].asBool(), false);
    EXPECT_EQ(data["secure"].asBool(), true);

    // Make a request as root user, which includes sysinfo.
    auto xml = R"(
<sysinfo>
</sysinfo>
)";
    EXPECT_CALL(lv, virConnectGetSysinfo(_, _)).WillOnce(Return(xml));

    endpoint = "/users/root/info/";
    location = make_location(R"(^/users/([^/]+)/info/)", endpoint);
    response_ = http::response();
    views_.show(conn_, location, request_, response_);

    data = json::parse(response_.body());
    EXPECT_EQ(data["hostname"].asString(), "test");
    EXPECT_EQ(data["libVersion"].asUInt(), 0);
    EXPECT_EQ(data["type"].asString(), "QEMU");
    EXPECT_EQ(data["uri"].asString(), "qemu+ssh://root@localhost/system");
    EXPECT_EQ(data["sysinfo"].type(), Json::objectValue);
    EXPECT_EQ(data["version"].asUInt(), 0);
    EXPECT_EQ(data["encrypted"].asBool(), false);
    EXPECT_EQ(data["secure"].asBool(), true);
    EXPECT_EQ(data["max_vcpus"].asUInt(), 2);
}

TEST_F(host_test, networks)
{
    std::vector<libvirt::network_ptr> networks = {
        std::make_shared<webvirt::network>()
    };
    EXPECT_CALL(lv, virConnectListAllNetworks(_, _))
        .WillOnce(Return(networks));

    auto xml = R"(
<network>
    <name>default</name>
    <uuid>1234-5678</uuid>
    <mac address="aa:bb:cc:dd:11:22:33:44" />
    <bridge delay="0" name="virbr0" stp="on" />
    <ip address="192.168.2.1" netmask="255.255.255.0">
        <dhcp>
            <range start="192.168.2.2" end="192.168.2.254" />
        </dhcp>
    </ip>
    <forward mode="nat">
        <nat>
            <port start="1024" end="65535" />
        </nat>
    </forward>
</network>
)";
    EXPECT_CALL(lv, virNetworkGetXMLDesc(_, _)).WillOnce(Return(xml));

    std::string endpoint("/users/test/networks/");
    auto location = make_location(R"(^/users/([^/]+)/networks/)", endpoint);
    views_.networks(conn_, location, request_, response_);

    auto data = json::parse(response_.body());
    auto network =
        data.get(Json::ArrayIndex(0), Json::Value(Json::objectValue));
    EXPECT_EQ(network["name"]["text"].asString(), "default");
    EXPECT_EQ(network["uuid"]["text"].asString(), "1234-5678");
    EXPECT_EQ(network["bridge"]["attrib"]["name"], "virbr0");
    EXPECT_EQ(network["ip"]["attrib"]["address"].asString(), "192.168.2.1");
    EXPECT_EQ(network["ip"]["attrib"]["netmask"].asString(), "255.255.255.0");
    EXPECT_EQ(network["mac"]["attrib"]["address"].asString(),
              "aa:bb:cc:dd:11:22:33:44");
}
