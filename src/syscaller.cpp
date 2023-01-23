/* Copyright (C) 2023 Kevin Morris <kevr@0cost.org> */
#include "syscaller.hpp"
#include <sys/stat.h>
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

uid_t syscaller::getuid()
{
    return ::getuid();
}

struct group *syscaller::getgrnam(const char *name)
{
    return ::getgrnam(name);
}

int syscaller::chown(const char *path, uid_t uid, gid_t gid)
{
    return ::chown(path, uid, gid);
}
