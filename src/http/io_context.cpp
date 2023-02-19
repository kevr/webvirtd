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
#include <http/io_context.hpp>

using namespace webvirt;

std::size_t http::io_context::run()
{
    return boost::asio::io_context::run();
}

void http::io_context::stop()
{
    boost::asio::post(*this, std::bind(&boost::asio::io_context::stop, this));
}
