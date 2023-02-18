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
#ifndef HTTP_HANDLERS_HPP
#define HTTP_HANDLERS_HPP

#include <http/types.hpp>

namespace webvirt::http
{

template <typename... args>
std::function<void(args...)> noop()
{
    return [](args...) {
    };
}

template <typename... args>
class handler : public std::function<void(args...)>
{
public:
    using std::function<void(args...)>::function;
    using type = std::function<void(args...)>;

    handler()
        : type(noop<args...>())
    {
    }

    handler(type f)
        : type(f)
    {
    }
};

}; // namespace webvirt::http

#define handler_setter(handler, member)                                       \
    void handler(decltype(member) fn)                                         \
    {                                                                         \
        member = fn;                                                          \
    }

#endif /* HTTP_HANDLERS_HPP */
