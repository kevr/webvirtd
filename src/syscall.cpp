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
#include <syscall.hpp>

#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace webvirt;

FILE *syscall::popen(const char *command, const char *mode)
{
    return ::popen(command, mode);
}

int syscall::pclose(FILE *stream)
{
    return ::pclose(stream);
}

char *syscall::fgets(char *buffer, int n, FILE *stream)
{
    return ::fgets(buffer, n, stream);
}

bool syscall::fs_remove(const std::filesystem::path &p)
{
    return std::filesystem::remove(p);
}

bool syscall::fs_remove_all(const std::filesystem::path &p)
{
    return std::filesystem::remove_all(p);
}

char *syscall::mkdtemp(char *template_)
{
    return ::mkdtemp(template_);
}

pid_t syscall::fork()
{
    return ::fork();
}

uid_t syscall::getuid()
{
    return ::getuid();
}

struct passwd *syscall::getpwuid(uid_t uid)
{
    return ::getpwuid(uid);
}

struct passwd *syscall::getpwnam(const char *name)
{
    return ::getpwnam(name);
}

gid_t syscall::getgid()
{
    return ::getgid();
}

struct group *syscall::getgrnam(const char *name)
{
    return ::getgrnam(name);
}

struct group *syscall::getgrgid(gid_t gid)
{
    return ::getgrgid(gid);
}

int syscall::chown(const char *path, uid_t uid, gid_t gid)
{
    return ::chown(path, uid, gid);
}

char *syscall::getenv(const char *name)
{
    return ::getenv(name);
}

void syscall::exit(int return_code)
{
    return ::exit(return_code);
}
