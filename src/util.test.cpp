/* Copyright (C) 2023 Kevin Morris <kevr@0cost.org */
#include "util.hpp"
#include "mocks/syscaller.hpp"
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

TEST_F(util_test, mkdir_fails)
{
    std::string tmpdir = "fake-tmpdir";
    EXPECT_CALL(sys, mkdtemp(_)).WillOnce(Return(tmpdir.data()));
    EXPECT_CALL(sys, mkdir(_, _)).WillOnce(Return(-1));
    EXPECT_THROW(webvirt::make_tmpdir(), std::runtime_error);
}
