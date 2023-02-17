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

    virtual char *mkdtemp(char *);

    virtual pid_t fork();

    virtual uid_t getuid();
    virtual struct passwd *getpwnam(const char *);
    virtual struct passwd *getpwuid(uid_t);

    virtual gid_t getgid();
    virtual struct group *getgrnam(const char *);
    virtual struct group *getgrgid(gid_t);

    virtual int chown(const char *, uid_t, gid_t);

    virtual char *getenv(const char *);

    virtual void exit(int);

private:
    static syscaller instance_;
    static syscaller *ptr_;

public:
    static syscaller &change(syscaller &);
    static syscaller &reset();
    static syscaller &ref();
};

}; // namespace webvirt

#endif /* SYSCALLER_HPP */
