/* Copyright (C) 2023 Kevin Morris <kevr@0cost.org> */
#ifndef SYSCALLER_HPP
#define SYSCALLER_HPP

#include <filesystem>
#include <grp.h>
#include <unistd.h>

namespace webvirt
{

class syscaller
{
public:
    virtual ~syscaller() = default;

public:
    virtual bool fs_remove(const std::filesystem::path &);
    virtual bool fs_remove_all(const std::filesystem::path &);

    virtual int mkdir(const char *, int);
    virtual char *mkdtemp(char *);

    virtual uid_t getuid();
    virtual struct group *getgrnam(const char *);
    virtual int chown(const char *, uid_t, gid_t);

private:
    static syscaller root_;
    static syscaller *ptr_;

public:
    static syscaller &change(syscaller *);
    static syscaller &reset();
    static syscaller &instance();
};

}; // namespace webvirt

#endif /* SYSCALLER_HPP */
