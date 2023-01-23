#include "mocks/syscaller.hpp"
#include "stubs/io_service.hpp"
#include "util.hpp"
#include <gtest/gtest.h>

#define main main_
#include "main.cpp"
#undef main

using testing::_;
using testing::Return;
using testing::Test;

class main_test : public Test
{
protected:
    static std::filesystem::path tmpdir, socket_path;

    webvirt::syscaller real_syscall;
    std::shared_ptr<webvirt::mocks::syscaller> syscall_ =
        std::make_shared<webvirt::mocks::syscaller>();

public:
    static void SetUpTestSuite()
    {
        tmpdir = socket_path = webvirt::make_tmpdir();
        socket_path /= "socket.sock";
    }

    void SetUp() override
    {
        auto &sys = webvirt::syscaller::instance();
        sys.fs_remove(socket_path);

        webvirt::syscaller::change(syscall_.get());
    }

    void TearDown() override
    {
        webvirt::syscaller::reset();
    }

    static void TestDownTestSuite()
    {
        auto &sys = webvirt::syscaller::instance();
        sys.fs_remove_all(tmpdir);
    }
};

std::filesystem::path main_test::tmpdir, main_test::socket_path;

TEST_F(main_test, must_be_root)
{
    auto &syscall = *syscall_.get();
    EXPECT_CALL(syscall, getuid()).WillOnce(Return(1000));

    const char *argv[] = { "webvirtd" };
    EXPECT_EQ(main_(1, argv), 1);
}

TEST_F(main_test, no_shadow_group)
{
    auto &syscall = *syscall_.get();
    EXPECT_CALL(syscall, getuid()).WillOnce(Return(0));
    EXPECT_CALL(syscall, fs_remove(_)).WillOnce(Return(true));
    EXPECT_CALL(syscall, getgrnam(_)).WillOnce(Return(nullptr));

    webvirt::io_service io;
    EXPECT_EQ(webvirt_main("webvirtd", io, socket_path), 2);
}

TEST_F(main_test, chown_fail)
{
    auto &syscall = *syscall_.get();
    EXPECT_CALL(syscall, getuid()).WillOnce(Return(0));
    EXPECT_CALL(syscall, fs_remove(_)).WillOnce(Return(true));

    struct group g;
    g.gr_gid = getgid();
    EXPECT_CALL(syscall, getgrnam(_)).WillOnce(Return(&g));

    EXPECT_CALL(syscall, chown(_, _, _)).WillOnce(Return(-1));

    webvirt::io_service io;
    EXPECT_EQ(webvirt_main("webvirtd", io, socket_path), 3);
}

TEST_F(main_test, runs)
{
    auto &syscall = *syscall_.get();
    EXPECT_CALL(syscall, getuid()).WillOnce(Return(0));
    EXPECT_CALL(syscall, fs_remove(_)).WillOnce(Return(true));

    struct group g;
    g.gr_gid = getgid();
    EXPECT_CALL(syscall, getgrnam(_)).WillOnce(Return(&g));

    EXPECT_CALL(syscall, chown(_, _, _)).WillOnce(Return(0));

    webvirt::stubs::io_service io;
    EXPECT_EQ(webvirt_main("webvirtd", io, socket_path), 0);
}
