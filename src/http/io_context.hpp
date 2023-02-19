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
#ifndef HTTP_IO_CONTEXT_HPP
#define HTTP_IO_CONTEXT_HPP

#include <boost/asio.hpp>

namespace webvirt::http
{

class io_context : public boost::asio::io_context
{
public:
    using boost::asio::io_context::io_context;
    virtual ~io_context() = default;

    virtual std::size_t run();
    virtual void stop();
};

}; // namespace webvirt::http

#endif /* HTTP_IO_CONTEXT_HPP */
