/* SPDX-License-Identifier: Apache 2.0 */
#include "util.hpp"
#include "mocks/syscaller.hpp"
#include "gtest/gtest.h"
#include <gtest/gtest.h>

using testing::_;
using testing::Return;
using testing::Test;

class util_test : public Test
{
protected:
    webvirt::mocks::syscaller sys;

public:
    void SetUp() override
    {
        webvirt::syscaller::change(&sys);
    }

    void TearDown() override
    {
        webvirt::syscaller::reset();
    }
};

TEST_F(util_test, mkdtemp_fails)
{
    EXPECT_CALL(sys, mkdtemp(_)).WillOnce(Return(nullptr));
    EXPECT_THROW(webvirt::make_tmpdir(), std::runtime_error);
}

TEST_F(util_test, println)
{
    testing::internal::CaptureStdout();
    webvirt::println("Test");
    auto stdout_ = testing::internal::GetCapturedStdout();
    EXPECT_EQ(stdout_, "Test\n");
}
