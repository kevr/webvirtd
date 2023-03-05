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
#include <util/logging.hpp>

#include <vector>

namespace webvirt::http
{

template <typename... args>
std::function<void(args...)> noop()
{
    return [](args...) {
    };
}

template <typename... args>
class handler
{
public:
    using type = std::function<void(args...)>;

private:
    std::vector<type> functions_;

public:
    handler() = default;

    handler(const handler &o)
        : functions_(o.functions_)
    {
    }

    handler &operator=(const handler &o)
    {
        functions_ = o.functions_;
        return *this;
    }

    void add(type f)
    {
        functions_.emplace_back(f);
    }

    void clear()
    {
        functions_.clear();
    }

    const std::vector<type> &functions() const
    {
        return functions_;
    }

    void operator()(args... args_) const
    {
        for (auto &f : functions_) {
            f(args_...);
        }
    }
};

}; // namespace webvirt::http

#define handler_setter(handler, member)                                       \
    void handler(typename decltype(member)::type fn)                          \
    {                                                                         \
        member.add(fn);                                                       \
    }                                                                         \
                                                                              \
    void handler(decltype(member) handler_)                                   \
    {                                                                         \
        member = handler_;                                                    \
    }                                                                         \
                                                                              \
    void handler##_override(typename decltype(member)::type fn)               \
    {                                                                         \
        member.clear();                                                       \
        member.add(fn);                                                       \
    }

#endif /* HTTP_HANDLERS_HPP */
