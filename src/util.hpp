/* SPDX-License-Identifier: Apache 2.0 */
#ifndef UTIL_HPP
#define UTIL_HPP

#include "syscaller.hpp"
#include <atomic>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <pwd.h>
#include <sys/fsuid.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

namespace webvirt
{

std::filesystem::path make_tmpdir();

template <typename T>
void print_stdout(const std::string &heading, const T &value)
{
    std::cout << "*** " << heading << " ***" << std::endl
              << value << std::endl
              << "*** END " << heading << " ***" << std::endl;
}

template <typename T>
void print_request(const T &value)
{
    print_stdout("REQUEST", value);
}

template <typename T>
void print_response(const T &value)
{
    print_stdout("RESPONSE", value);
}

template <typename T>
int print(const T &message)
{
    std::cout << message;
    return 0;
}

int println(std::string);
int error(std::string, int return_code = 1);
int errorln(std::string, int return_code = 1);

}; // namespace webvirt

#endif /* UTIL_HPP */
