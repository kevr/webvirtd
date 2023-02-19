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
#include <util/logging.hpp>
#include <util/signal.hpp>

#include <gtest/gtest.h>

using namespace webvirt;

using testing::Test;

class signal_test : public Test
{
public:
    void SetUp() override
    {
        logger::enable_debug(true);
    }

    void TearDown() override
    {
        logger::reset_debug();
    }
};

TEST_F(signal_test, sigpipe)
{
    testing::internal::CaptureStdout();
    signal::pipe(SIGPIPE);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("Caught SIGPIPE"), std::string::npos);
}

TEST_F(signal_test, sigint)
{
    testing::internal::CaptureStdout();
    signal::int_(SIGINT);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("Caught SIGINT"), std::string::npos);
}
