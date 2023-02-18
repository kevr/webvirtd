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
#ifndef UTIL_RETRY_HPP
#define UTIL_RETRY_HPP

#include <functional>
#include <stdexcept>

namespace webvirt
{

struct retry_error : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

template <typename exception = retry_error,
          typename fatal_exception = std::domain_error>
class retry
{
private:
    unsigned int retries_ { 1 };
    unsigned int retry_ { 0 };
    std::function<void()> func_;

public:
    retry(std::function<void()> func)
        : func_(func)
    {
    }

    retry &retries(unsigned int n)
    {
        retries_ = n;
        return *this;
    }

    void operator()()
    {
        try {
            func_();
        } catch (const exception &exc) {
            if (retry_++ < retries_) {
                operator()();
            } else {
                throw fatal_exception(exc.what());
            }
        }
    }
};

}; // namespace webvirt

#endif /* UTIL_RETRY_HPP */
