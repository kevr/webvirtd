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
#include <http/handlers.hpp>
#include <util/logging.hpp>

using namespace webvirt;

void http::on_close()
{
}

void http::on_error(const char *source, boost::beast::error_code ec)
{
    std::string message(source);
    message.append(": ");
    message.append(ec.message());
    logger::error(message);
}

void http::on_response(
    const boost::beast::http::response<boost::beast::http::string_body> &)
{
}
