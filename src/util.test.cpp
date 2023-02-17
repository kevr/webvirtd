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
#include "util.hpp"
#include "mocks/syscaller.hpp"
#include <gtest/gtest.h>
using namespace webvirt;

using testing::_;
using testing::Return;
using testing::Test;

class util_test : public Test
{
protected:
    mocks::syscaller sys;

public:
    void SetUp() override
    {
        syscaller::change(sys);
    }

    void TearDown() override
    {
        syscaller::reset();
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
