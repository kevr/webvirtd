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
