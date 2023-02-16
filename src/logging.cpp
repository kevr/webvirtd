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
#include "logging.hpp"
#include "config.hpp"
#include <fmt/format.h>
using namespace webvirt;

std::atomic<bool> logger::debug_ { false };
std::atomic<bool> logger::time_ { true };

void logger::info(const std::string &message)
{
    return print(std::cout, "[INFO]", message);
}

void logger::error(const std::string &message)
{
    return print(std::cerr, "[ERROR]", message);
}

void logger::debug(const std::string &message)
{
    if (debug_) {
        print(std::cout, "[DEBUG]", message);
    }
}

void logger::debug(std::function<std::string()> action)
{
    if (debug_) {
        debug(action());
    }
}

void logger::enable_timestamp(bool enabled)
{
    time_ = enabled;
}

void logger::reset_timestamp()
{
    enable_timestamp(true);
}

void logger::enable_debug(bool enabled)
{
    debug_ = enabled;
}

void logger::reset_debug()
{
    enable_debug(false);
}

std::string logger::timestamp()
{
    auto now = std::time({});
    char format[std::size("dd/Mon/yyyy hh:mm:ss")];
    std::strftime(std::data(format),
                  std::size(format),
                  "%d/%b/%Y %H:%M:%S",
                  std::localtime(&now));
    return format;
}

void logger::print(std::ostream &os, const std::string &prefix,
                   const std::string &message)
{
    std::string ts;
    if (time_) {
        ts = "[" + timestamp() + "] ";
    }
    os << fmt::format("{}{} {}\n", ts, prefix, message);
}
