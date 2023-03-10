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
#include <mocks/syscall.hpp>
#include <stubs/io_context.hpp>
#include <util/util.hpp>

#include <gtest/gtest.h>

#define main main_
#include <main.cpp>
#undef main

using namespace webvirt;

using testing::_;
using testing::Invoke;
using testing::Return;
using testing::Test;

class main_test : public Test
{
protected:
    static std::filesystem::path tmpdir, socket_path;
    webvirt::syscall sys;
    config conf;

public:
    static void SetUpTestSuite()
    {
        tmpdir = socket_path = webvirt::make_tmpdir();
        socket_path /= "socket.sock";
    }

    virtual void SetUp() override
    {
        conf.add_option("socket,s",
                        boost::program_options::value<std::string>()
                            ->default_value(socket_path)
                            ->multitoken(),
                        "unix socket path");
        conf.add_option("threads",
                        boost::program_options::value<unsigned>()
                            ->default_value(1)
                            ->multitoken(),
                        "number of worker threads");

        auto gid = sys.getgid();
        auto *default_group = sys.getgrgid(gid);
        conf.add_option("socket-group",
                        boost::program_options::value<std::string>()
                            ->default_value(default_group->gr_name)
                            ->multitoken(),
                        "socket group");

        const char *argv[] = { "webvirtd" };
        conf.parse(1, argv);
    }

    void TearDown() override
    {
        webvirt::syscall::reset();
        webvirt::config::reset();
    }

    static void TearDownTestSuite()
    {
        webvirt::syscall().fs_remove_all(tmpdir);
    }
};

class webvirt_main_test : public main_test
{
protected:
    webvirt::stubs::io_context io { 0 };

public:
    static void SetUpTestSuite()
    {
        main_test::SetUpTestSuite();
    }

    void SetUp() override
    {
        main_test::SetUp();
        webvirt::config::change(conf);
    }

    void TearDown() override
    {
        main_test::TearDown();
    }

    static void TearDownTestSuite()
    {
        main_test::TearDownTestSuite();
    }
};

std::filesystem::path main_test::tmpdir;
std::filesystem::path main_test::socket_path;

TEST_F(webvirt_main_test, version)
{
    testing::internal::CaptureStdout();

    const char *argv[] = { "webvirtd", "--version" };
    EXPECT_EQ(main_(2, argv), 0);

    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find(VERSION), std::string::npos);
}

TEST_F(webvirt_main_test, runs)
{
    EXPECT_EQ(webvirt_main(io, socket_path), 0);
}

TEST_F(webvirt_main_test, group_not_found)
{
    mocks::syscall sys;
    syscall::change(sys);

    EXPECT_CALL(sys, fs_remove(_)).WillOnce(Invoke([this](const auto &arg) {
        return this->sys.fs_remove(arg);
    }));
    EXPECT_CALL(sys, getgrnam(_)).WillOnce(Return(nullptr));
    EXPECT_EQ(webvirt_main(io, socket_path), 2);
}

TEST_F(webvirt_main_test, chown_failed)
{
    mocks::syscall sys;
    syscall::change(sys);

    EXPECT_CALL(sys, fs_remove(_)).WillOnce(Invoke([this](const auto &arg) {
        return this->sys.fs_remove(arg);
    }));

    struct group g;
    g.gr_gid = 0;
    EXPECT_CALL(sys, getgrnam(_)).WillOnce(Return(&g));
    EXPECT_CALL(sys, getuid()).WillOnce(Return(0));
    EXPECT_CALL(sys, chown(_, _, _)).WillOnce(Return(-1));

    EXPECT_EQ(webvirt_main(io, socket_path), 3);
}

TEST_F(main_test, unknown_option)
{
    const char *argv[] = { "webvirtd", "--blahblah" };
    EXPECT_EQ(main_(2, argv), 1);
}

TEST_F(main_test, help)
{
    const char *argv[] = { "webvirtd", "--help" };
    EXPECT_EQ(main_(2, argv), 0);
}

TEST_F(main_test, runs)
{
    mocks::syscall sys;
    syscall::change(sys);

    EXPECT_CALL(sys, getgid()).WillOnce(Return(this->sys.getgid()));
    EXPECT_CALL(sys, fs_remove(_)).WillOnce(Invoke([this](const auto &path) {
        return this->sys.fs_remove(path);
    }));

    // Mock getgrnam to return no group; this causes webvirt_main to
    // exit out with return code 2, stopping execution before io_service
    // is run. The point of this test is to exercise main()'s code,
    // which is already executed before the point getgrname() is called.
    EXPECT_CALL(sys, getgrnam(_)).WillOnce(Return(nullptr));

    const char *argv[] = { "webvirtd", "--socket", socket_path.c_str() };
    EXPECT_EQ(main_(3, argv), 2);
}
