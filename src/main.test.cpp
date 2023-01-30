#include "mocks/syscaller.hpp"
#include "stubs/io_service.hpp"
#include "util.hpp"
#include <gtest/gtest.h>

#define main main_
#include "main.cpp"
#undef main

using testing::_;
using testing::Invoke;
using testing::Return;
using testing::Test;

class main_test : public Test
{
protected:
public:
    virtual void SetUp() override
    {
    }
};

class webvirt_main_test : public main_test
{
protected:
    static std::filesystem::path tmpdir, socket_path;

    webvirt::syscaller sys;
    webvirt::config conf;
    webvirt::stubs::io_service io { 0 };

public:
    static void SetUpTestSuite()
    {
        tmpdir = socket_path = webvirt::make_tmpdir();
        socket_path /= "socket.sock";
    }

    void SetUp() override
    {
        auto gid = sys.getgid();
        auto *default_group = sys.getgrgid(gid);
        conf.add_option("socket-group",
                        boost::program_options::value<std::string>()
                            ->default_value(default_group->gr_name)
                            ->multitoken(),
                        "socket group");

        webvirt::config::change(conf);
        const char *argv[] = { "webvirtd" };
        conf.parse(1, argv);
    }

    void TearDown() override
    {
        webvirt::syscaller::reset();
        webvirt::config::reset();
    }

    static void TearDownTestSuite()
    {
        webvirt::syscaller().fs_remove_all(tmpdir);
    }
};

std::filesystem::path webvirt_main_test::tmpdir;
std::filesystem::path webvirt_main_test::socket_path;

TEST_F(webvirt_main_test, runs)
{
    EXPECT_EQ(webvirt_main(io, socket_path), 0);
}

TEST_F(webvirt_main_test, group_not_found)
{
    webvirt::mocks::syscaller sys;
    webvirt::syscaller::change(&sys);

    EXPECT_CALL(sys, fs_remove(_)).WillOnce(Invoke([this](const auto &arg) {
        return this->sys.fs_remove(arg);
    }));
    EXPECT_CALL(sys, getgrnam(_)).WillOnce(Return(nullptr));
    EXPECT_EQ(webvirt_main(io, socket_path), 2);
}

TEST_F(webvirt_main_test, chown_failed)
{
    webvirt::mocks::syscaller sys;
    webvirt::syscaller::change(&sys);

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
