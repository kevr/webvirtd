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
#include "config.hpp"
#include "util.hpp"
#include <fstream>
#include <gtest/gtest.h>
#include <syscaller.hpp>
using namespace webvirt;

using testing::Test;

class config_test : public Test
{
protected:
    config conf;
};

TEST_F(config_test, constructors)
{
    {
        config copied(conf);
    }

    config moved(std::move(conf));
}

TEST_F(config_test, add_option)
{
    conf.add_option("test-option", "a test option");

    const char *argv[] = { "webvirtd", "--test-option" };
    conf.parse(2, argv);

    EXPECT_TRUE(conf.has("test-option"));
}

TEST_F(config_test, parse_file)
{
    auto tmpdir = make_tmpdir();
    auto config_file = tmpdir / "webvirtd.conf";

    conf.add_option("test-option",
                    boost::program_options::value<std::string>(),
                    "test option");

    std::ofstream config_stream(config_file, std::ios::out);
    config_stream << "test-option = test-value\n";
    config_stream.close();
    conf.parse(config_file);
    syscaller::ref().fs_remove_all(tmpdir);

    EXPECT_EQ(conf.get<std::string>("test-option"), "test-value");
}
