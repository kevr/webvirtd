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
#include <virt/events/lifecycle.hpp>

#include <gtest/gtest.h>

using namespace webvirt;
using namespace virt;

using testing::_;
using testing::Return;
using testing::Test;

class lifecycle_test : public Test
{
protected:
    mocks::libvirt lv;

    webvirt::connect_ptr ptr_;
    virt::connection conn_;

public:
    void SetUp() override
    {
        libvirt::change(lv);

        ptr_ = std::make_shared<webvirt::connect>();
        EXPECT_CALL(lv, virConnectOpen(_)).WillOnce(Return(ptr_));

        conn_.connect("qemu+ssh://test@localhost/session");
    }

    void TearDown() override
    {
        libvirt::reset();
    }
};

TEST_F(lifecycle_test, register_fails)
{
    lifecycle_callback cb(lifecycle_event::on_event_handler);

    EXPECT_CALL(lv, virConnectDomainEventRegisterAny(_, _, _, _, _, _))
        .WillOnce(Return(-1));
    EXPECT_THROW(
        {
            lifecycle_event(
                conn_,
                cb,
                http::noop<virt::connection &, virt::domain &, int, int>());
        },
        std::runtime_error);
}
