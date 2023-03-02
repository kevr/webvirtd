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
#ifndef UTIL_LOGGING_HPP
#define UTIL_LOGGING_HPP

#include <atomic>
#include <chrono>
#include <fmt/format.h>
#include <functional>
#include <iostream>
#include <string>

namespace webvirt
{

class logger
{
private:
    static std::atomic<bool> debug_;
    static std::atomic<bool> time_;

public:
    static void info(const std::string &);
    static void error(const std::string &);
    static void debug(const std::string &);
    static void debug(std::function<std::string()>);

public:
    static void enable_timestamp(bool);
    static void reset_timestamp();
    static void enable_debug(bool);
    static void reset_debug();

private:
    static std::string timestamp();
    static void print(std::ostream &, const std::string &,
                      const std::string &);
};

/** Extract 'namespace::classname' from __PRETTY_FUNCTION__ text
 *
 * @param pretty_function __PRETTY_FUNCTION__
 * @returns 'namespace::classname' from __PRETTY_FUNCTION__
 **/
std::string pretty_function_prefix(const std::string &);

}; // namespace webvirt

#define CLASS_TRACE(message)                                                  \
    webvirt::logger::debug(                                                   \
        fmt::format("[{}] {}",                                                \
                    webvirt::pretty_function_prefix(__PRETTY_FUNCTION__),     \
                    message));

#define CLASS_ETRACE(message)                                                 \
    webvirt::logger::error(                                                   \
        fmt::format("[{}] {}",                                                \
                    webvirt::pretty_function_prefix(__PRETTY_FUNCTION__),     \
                    message));

#endif /* UTIL_LOGGING_HPP */
