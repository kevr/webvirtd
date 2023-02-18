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
#include <mocks/libvirt.hpp>
#include <virt/connection.hpp>

#include <cstring>
#include <gtest/gtest.h>

using namespace webvirt;

using testing::_;
using testing::Return;
using testing::Test;

class connection_test : public Test
{
protected:
    mocks::libvirt lv;
    connect_ptr conn = std::make_shared<webvirt::connect>();

public:
    void SetUp() override
    {
        libvirt::change(lv);
        EXPECT_CALL(lv, virConnectOpen(_)).WillOnce(Return(conn));
    }

    void TearDown() override
    {
        libvirt::reset();
    }
};

TEST_F(connection_test, cannot_connect_twice)
{
    virt::connection v("/path/to/socket.sock");
    EXPECT_THROW(v.connect("/path/to/socket.sock"), std::overflow_error);
}

TEST_F(connection_test, copy)
{
    virt::connection v("/path/to/socket.sock");

    virt::connection v2(v);
    EXPECT_EQ(v, v2);

    v = v2;
    EXPECT_EQ(v, v2);
}

TEST_F(connection_test, inequality)
{
    virt::connection v("/path/to/socket.sock");
    EXPECT_NE(v, virt::connection());
}

TEST(connection, on_libvirt_error)
{
    testing::internal::CaptureStderr();

    webvirt::logger log;
    webvirt::error__ err;
    std::string str("test error");
    err.message = str.data();
    virt::on_libvirt_error(&log, &err);

    auto output = testing::internal::GetCapturedStderr();
    EXPECT_NE(output.find("test error"), std::string::npos);
}

TEST(connection, on_libvirt_error_metadata)
{
    testing::internal::CaptureStderr();

    webvirt::logger log;
    webvirt::error__ err;
    std::string str("Requested metadata element is not present");
    err.message = str.data();
    virt::on_libvirt_error(&log, &err);

    auto output = testing::internal::GetCapturedStderr();
    EXPECT_EQ(output, "");
}
