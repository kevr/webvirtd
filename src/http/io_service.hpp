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
#ifndef HTTP_IO_SERVICE_HPP
#define HTTP_IO_SERVICE_HPP

#include <boost/asio.hpp>

namespace webvirt
{

/**
 * A boost::asio::io_service wrapper
 **/
class io_service : public boost::asio::io_service
{
public:
    using boost::asio::io_service::io_service;
    virtual ~io_service() = default;

    virtual std::size_t process();
};

}; // namespace webvirt

#endif /* HTTP_IO_SERVICE_HPP */
