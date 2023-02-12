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
#include <gtest/gtest.h>
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
    std::smatch make_location(const std::string &pattern,
                              const std::string &uri)
    {
        std::regex re(pattern);
        std::smatch m;
        std::regex_match(uri, m, re);
        return m;
    }
};

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

    auto location = make_location(R"(^/users/([^/]+)/networks/$)",
                                  "/users/test/networks/");
    request_.method(beast::http::verb::get);
    views_.networks(conn_, location, request_, response_);
}
