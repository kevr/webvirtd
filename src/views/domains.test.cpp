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
#include "../json.hpp"
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

TEST_F(domains_test, metadata_not_found)
{
    EXPECT_CALL(lv, virDomainLookupByName(_, _))
        .WillRepeatedly(Return(nullptr));

    request_.method(boost::beast::http::verb::post);

    auto location =
        make_location(R"(^/users/([^/]+)/domains/([^/]+)/metadata/$)",
                      "/users/test/domains/test/metadata/");
    views_.metadata(conn_, "test", location, request_, response_);

    EXPECT_EQ(response_.result(), beast::http::status::not_found);
}

TEST_F(domains_test, metadata_bad_request)
{
    libvirt::domain_ptr domain_ptr = std::make_shared<libvirt::domain>();
    EXPECT_CALL(lv, virDomainLookupByName(_, _))
        .WillRepeatedly(Return(domain_ptr));

    request_.method(boost::beast::http::verb::post);

    auto location =
        make_location(R"(^/users/([^/]+)/domains/([^/]+)/metadata/$)",
                      "/users/test/domains/test/metadata/");
    views_.metadata(conn_, "test", location, request_, response_);

    EXPECT_EQ(response_.result(), beast::http::status::bad_request);
}

TEST_F(domains_test, metadata_title)
{
    libvirt::domain_ptr domain_ptr = std::make_shared<libvirt::domain>();
    EXPECT_CALL(lv, virDomainLookupByName(_, _))
        .WillRepeatedly(Return(domain_ptr));

    EXPECT_CALL(lv, virDomainSetMetadata(_, _, _, _, _, _))
        .WillOnce(Return(0));
    EXPECT_CALL(lv, virDomainGetMetadata(_, _, _, _))
        .WillOnce(Return(""))
        .WillRepeatedly(Return("Test Title"));

    request_.method(boost::beast::http::verb::post);

    Json::Value data(Json::objectValue);
    data["title"] = "Test Title";
    boost::beast::ostream(request_.body()) << json::stringify(data);
    request_.content_length(request_.body().size());

    auto location =
        make_location(R"(^/users/([^/]+)/domains/([^/]+)/metadata/$)",
                      "/users/test/domains/test/metadata/");
    views_.metadata(conn_, "test", location, request_, response_);

    auto domain = conn_.domain("test");
    auto title = domain.metadata(VIR_DOMAIN_METADATA_TITLE, nullptr, 0);
    EXPECT_EQ(title, "Test Title");

    EXPECT_EQ(response_.result(), beast::http::status::ok);

    views_.metadata(conn_, "test", location, request_, response_);
    EXPECT_EQ(response_.result(), beast::http::status::not_modified);
}

TEST_F(domains_test, metadata_description)
{
    libvirt::domain_ptr domain_ptr = std::make_shared<libvirt::domain>();
    EXPECT_CALL(lv, virDomainLookupByName(_, _))
        .WillRepeatedly(Return(domain_ptr));

    EXPECT_CALL(lv, virDomainSetMetadata(_, _, _, _, _, _))
        .WillOnce(Return(0));
    EXPECT_CALL(lv, virDomainGetMetadata(_, _, _, _))
        .WillOnce(Return(""))
        .WillOnce(Return(""))
        .WillRepeatedly(Return("Test description."));

    request_.method(boost::beast::http::verb::post);

    Json::Value data(Json::objectValue);
    data["description"] = "Test description.";
    boost::beast::ostream(request_.body()) << json::stringify(data);
    request_.content_length(request_.body().size());

    auto location =
        make_location(R"(^/users/([^/]+)/domains/([^/]+)/metadata/$)",
                      "/users/test/domains/test/metadata/");
    views_.metadata(conn_, "test", location, request_, response_);

    auto domain = conn_.domain("test");
    auto title = domain.metadata(VIR_DOMAIN_METADATA_DESCRIPTION, nullptr, 0);
    EXPECT_EQ(title, "Test description.");

    EXPECT_EQ(response_.result(), beast::http::status::ok);

    views_.metadata(conn_, "test", location, request_, response_);
    EXPECT_EQ(response_.result(), beast::http::status::not_modified);
}
