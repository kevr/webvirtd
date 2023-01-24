/* Copyright (C) 2023 Kevin Morris <kevr@0cost.org> */
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
    return instance();
}

syscaller &syscaller::reset()
{
    return change(&root_);
}

syscaller &syscaller::instance()
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

int syscaller::mkdir(const char *path, int permissions)
{
    return ::mkdir(path, permissions);
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

int syscaller::setuid(uid_t uid)
{
    return ::setuid(uid);
}

struct passwd *syscaller::getpwuid(uid_t uid)
{
    return ::getpwuid(uid);
}

struct passwd *syscaller::getpwnam(const char *name)
{
    return ::getpwnam(name);
}

int syscaller::setgid(gid_t gid)
{
    return ::setgid(gid);
}

struct group *syscaller::getgrnam(const char *name)
{
    return ::getgrnam(name);
}

int syscaller::chown(const char *path, uid_t uid, gid_t gid)
{
    return ::chown(path, uid, gid);
}

char *syscaller::getenv(const char *name)
{
    return ::getenv(name);
}

int syscaller::setenv(const char *name, const char *value, int replace)
{
    return ::setenv(name, value, replace);
}

pid_t syscaller::waitpid(pid_t pid, int *status, int options)
{
    return ::waitpid(pid, status, options);
}

void syscaller::exit(int return_code)
{
    return ::exit(return_code);
}
