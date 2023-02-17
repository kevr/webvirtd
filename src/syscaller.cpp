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
#include "syscaller.hpp"
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
using namespace webvirt;

syscaller syscaller::root_;
syscaller *syscaller::ptr_ = &syscaller::root_;

syscaller &syscaller::change(syscaller *ptr)
{
    ptr_ = ptr;
    return ref();
}

syscaller &syscaller::reset()
{
    return change(&root_);
}

syscaller &syscaller::ref()
{
    return *ptr_;
}

FILE *syscaller::popen(const char *command, const char *mode)
{
    return ::popen(command, mode);
}

int syscaller::pclose(FILE *stream)
{
    return ::pclose(stream);
}

char *syscaller::fgets(char *buffer, int n, FILE *stream)
{
    return ::fgets(buffer, n, stream);
}

bool syscaller::fs_remove(const std::filesystem::path &p)
{
    return std::filesystem::remove(p);
}

bool syscaller::fs_remove_all(const std::filesystem::path &p)
{
    return std::filesystem::remove_all(p);
}

char *syscaller::mkdtemp(char *template_)
{
    return ::mkdtemp(template_);
}

pid_t syscaller::fork()
{
    return ::fork();
}

uid_t syscaller::getuid()
{
    return ::getuid();
}

struct passwd *syscaller::getpwuid(uid_t uid)
{
    return ::getpwuid(uid);
}

struct passwd *syscaller::getpwnam(const char *name)
{
    return ::getpwnam(name);
}

gid_t syscaller::getgid()
{
    return ::getgid();
}

struct group *syscaller::getgrnam(const char *name)
{
    return ::getgrnam(name);
}

struct group *syscaller::getgrgid(gid_t gid)
{
    return ::getgrgid(gid);
}

int syscaller::chown(const char *path, uid_t uid, gid_t gid)
{
    return ::chown(path, uid, gid);
}

char *syscaller::getenv(const char *name)
{
    return ::getenv(name);
}

void syscaller::exit(int return_code)
{
    return ::exit(return_code);
}
