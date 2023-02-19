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
#ifndef STUBS_IO_CONTEXT_HPP
#define STUBS_IO_CONTEXT_HPP

#include <http/io_context.hpp>

namespace webvirt::stubs
{

class io_context : public http::io_context
{
private:
    std::size_t iterations = 0;

public:
    io_context(std::size_t iterations = 0);
    std::size_t run() override;
};

}; // namespace webvirt::stubs

#endif /* STUBS_IO_CONTEXT_HPP */
