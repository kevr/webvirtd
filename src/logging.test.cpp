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
#include "logging.hpp"
#include "http/io_service.hpp"
#include "gtest/gtest.h"
#include <gtest/gtest.h>
#include <regex>
using namespace webvirt;

using testing::Test;

class logger_test : public Test
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

TEST_F(logger_test, debug)
{
    testing::internal::CaptureStdout();
    logger::debug("Test");
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("[DEBUG] Test"), std::string::npos);
}

TEST_F(logger_test, lazy_debug)
{
    testing::internal::CaptureStdout();
    logger::debug([] {
        return "Test";
    });
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("[DEBUG] Test"), std::string::npos);
}

TEST_F(logger_test, enable_timestamp)
{
    testing::internal::CaptureStdout();

    // By default, enable_timestamp is true.
    logger::info("Test");
    std::string output = testing::internal::GetCapturedStdout();
    std::regex re(
        R"(\[[0-9]{2}/[a-zA-Z]{3}/[0-9]{4} [0-9]{2}:[0-9]{2}:[0-9]{2}\])");
    EXPECT_TRUE(std::regex_search(output, re));

    logger::enable_timestamp(false);
    testing::internal::CaptureStdout();
    logger::info("Test");
    output = testing::internal::GetCapturedStdout();
    EXPECT_FALSE(std::regex_search(output, re));

    logger::reset_timestamp();
    testing::internal::CaptureStdout();
    logger::info("Test");
    output = testing::internal::GetCapturedStdout();
    EXPECT_TRUE(std::regex_search(output, re));
}
