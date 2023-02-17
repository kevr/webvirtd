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
#include "util.hpp"
#include "config.hpp"
#include "syscall.hpp"
#include <cstring>
#include <filesystem>
#include <stdexcept>
#include <sys/stat.h>
#include <unistd.h>
using namespace webvirt;

std::filesystem::path webvirt::make_tmpdir()
{
    auto &sys = syscall::ref();
    std::string template_("/tmp/webvirtd-XXXXXX");

    char *result = sys.mkdtemp(template_.data());
    if (!result) {
        std::string error("mkdtemp errno = " + std::to_string(errno) + " (" +
                          strerror(errno) + ")");
        throw std::runtime_error(error.c_str());
    }

    return result;
}

int webvirt::println(std::string message)
{
    message.push_back('\n');
    return print(std::move(message));
}

int webvirt::error(std::string message, int return_code)
{
    message = "error: " + message;
    std::cerr << message;
    return return_code;
}

int webvirt::errorln(std::string message, int return_code)
{
    message.push_back('\n');
    return error(std::move(message), return_code);
}
