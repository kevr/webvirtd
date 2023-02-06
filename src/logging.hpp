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
#ifndef LOGGING_HPP
#define LOGGING_HPP

#include <chrono>
#include <iostream>
#include <string>

namespace webvirt
{

class logger
{
private:
    std::chrono::system_clock clock_;

public:
    void info(const std::string &);
    void error(const std::string &);

private:
    std::string timestamp();
    void print(std::ostream &, const std::string &, const std::string &);
};

}; // namespace webvirt

#endif /* LOGGING_HPP */
