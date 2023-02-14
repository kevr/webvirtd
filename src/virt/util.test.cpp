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
#include <gtest/gtest.h>
using namespace webvirt;

TEST(util, root_uri)
{
    EXPECT_EQ(virt::uri("root"), "qemu+ssh://root@localhost/system");
}

TEST(util, user_uri)
{
    EXPECT_EQ(virt::uri("test"), "qemu+ssh://test@localhost/session");
}
