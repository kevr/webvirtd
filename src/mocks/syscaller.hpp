/* SPDX-License-Identifier: Apache 2.0 */
#ifndef MOCKS_SYSCALLER_HPP
#define MOCKS_SYSCALLER_HPP

#include "../syscaller.hpp"
#include <filesystem>
#include <gmock/gmock.h>
#include <grp.h>
#include <unistd.h>

namespace webvirt::mocks
{

class syscaller : public webvirt::syscaller
{
public:
    virtual ~syscaller() = default;

public:
    MOCK_METHOD(bool, fs_remove, (const std::filesystem::path &));
    MOCK_METHOD(int, mkdir, (const char *, int));
    MOCK_METHOD(char *, mkdtemp, (char *));
    MOCK_METHOD(uid_t, getuid, ());
    MOCK_METHOD(struct group *, getgruid, (const char *));
    MOCK_METHOD(struct group *, getgrnam, (const char *));
    MOCK_METHOD(gid_t, getgid, ());
    MOCK_METHOD(int, chown, (const char *, uid_t, gid_t));
};

}; // namespace webvirt::mocks

#endif /* MOCKS_SYSCALLER_HPP */
