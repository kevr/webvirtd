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
#ifndef MOCKS_SYSCALLER_HPP
#define MOCKS_SYSCALLER_HPP

#include "../syscall.hpp"
#include <filesystem>
#include <gmock/gmock.h>
#include <grp.h>
#include <unistd.h>

namespace webvirt::mocks
{

class syscall : public webvirt::syscall
{
public:
    virtual ~syscall() = default;

public:
    MOCK_METHOD(bool, fs_remove, (const std::filesystem::path &));
    MOCK_METHOD(char *, mkdtemp, (char *));
    MOCK_METHOD(struct passwd *, getpwnam, (const char *));
    MOCK_METHOD(uid_t, getuid, ());
    MOCK_METHOD(struct group *, getgruid, (uid_t));
    MOCK_METHOD(struct group *, getgrnam, (const char *));
    MOCK_METHOD(gid_t, getgid, ());
    MOCK_METHOD(int, chown, (const char *, uid_t, gid_t));
};

}; // namespace webvirt::mocks

#endif /* MOCKS_SYSCALLER_HPP */
