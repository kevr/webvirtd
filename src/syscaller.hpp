/* SPDX-License-Identifier: Apache 2.0 */
#ifndef SYSCALLER_HPP
#define SYSCALLER_HPP

#include <filesystem>
#include <grp.h>
#include <pwd.h>
#include <unistd.h>

namespace webvirt
{

class syscaller
{
public:
    virtual ~syscaller() = default;

public:
    virtual FILE *popen(const char *, const char *);
    virtual int pclose(FILE *);

    virtual char *fgets(char *, int, FILE *);

    virtual bool fs_remove(const std::filesystem::path &);
    virtual bool fs_remove_all(const std::filesystem::path &);

    virtual int mkdir(const char *, int);
    virtual char *mkdtemp(char *);

    virtual pid_t fork();

    virtual uid_t getuid();
    virtual int setuid(uid_t);
    virtual struct passwd *getpwnam(const char *);
    virtual struct passwd *getpwuid(uid_t);

    virtual gid_t getgid();
    virtual int setgid(gid_t);
    virtual struct group *getgrnam(const char *);
    virtual struct group *getgrgid(gid_t);

    virtual int chown(const char *, uid_t, gid_t);

    virtual char *getenv(const char *);
    virtual int setenv(const char *, const char *, int);

    virtual pid_t waitpid(pid_t, int *, int);

    virtual void exit(int);

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
