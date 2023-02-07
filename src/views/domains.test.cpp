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
#include "domains.hpp"
#include "../mocks/libvirt.hpp"
#include "../virt/connection.hpp"
#include <gtest/gtest.h>
using namespace webvirt;

using testing::_;
using testing::Invoke;
using testing::Return;
using testing::Test;

class domains_test : public Test
{
protected:
    mocks::libvirt lv;

    virt::connection conn_;
    http::request request_;
    http::response response_;

    views::domains views_;

public:
    void SetUp() override
    {
        libvirt::change(lv);
        libvirt::connect_ptr ptr = std::make_shared<libvirt::connect>();
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

TEST_F(domains_test, autostart_post)
{
    libvirt::domain_ptr domain = std::make_shared<libvirt::domain>();
    EXPECT_CALL(lv, virDomainLookupByName(_, _)).WillOnce(Return(domain));

    int autostart = 0;
    EXPECT_CALL(lv, virDomainSetAutostart(_, _))
        .WillOnce(Invoke([&](auto, int autostart_) {
            autostart = autostart_;
            return 0;
        }));

    auto location =
        make_location(R"(^/users/([^/]+)/domains/([^/]+)/autostart/$)",
                      "/users/test/domains/test/autostart/");
    request_.method(boost::beast::http::verb::post);
    views_.autostart(conn_, "test", location, request_, response_);

    EXPECT_EQ(response_.result(), boost::beast::http::status::ok);
    EXPECT_EQ(autostart, 1);
}

TEST_F(domains_test, autostart_delete)
{
    libvirt::domain_ptr domain = std::make_shared<libvirt::domain>();
    EXPECT_CALL(lv, virDomainLookupByName(_, _)).WillOnce(Return(domain));

    int autostart = 1;
    EXPECT_CALL(lv, virDomainSetAutostart(_, _))
        .WillOnce(Invoke([&](auto, int autostart_) {
            autostart = autostart_;
            return 0;
        }));

    auto location =
        make_location(R"(^/users/([^/]+)/domains/([^/]+)/autostart/$)",
                      "/users/test/domains/test/autostart/");
    request_.method(boost::beast::http::verb::delete_);
    views_.autostart(conn_, "test", location, request_, response_);

    EXPECT_EQ(response_.result(), boost::beast::http::status::ok);
    EXPECT_EQ(autostart, 0);
}

TEST_F(domains_test, autostart_not_found)
{
    EXPECT_CALL(lv, virDomainLookupByName(_, _)).WillOnce(Return(nullptr));

    auto location =
        make_location(R"(^/users/([^/]+)/domains/([^/]+)/autostart/$)",
                      "/users/test/domains/test/autostart/");
    views_.autostart(conn_, "test", location, request_, response_);

    EXPECT_EQ(response_.result(), boost::beast::http::status::not_found);
}
